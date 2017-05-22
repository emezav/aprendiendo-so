/**
* @file
* @ingroup kernel_code
* @author Erwin Meza <emezav@gmail.com>
* @copyright GNU Public License.
* @brief Sector de Arranque en Imagen de Floppy
* @details

* @details
* Este sector de arranque se encarga de 'leer' un kernel en formato
* binario que se encuentra almacenado a partir del segundo sector de la imagen
* de disco. Este kernel se carga en la dirección de memoria 0x8000, es decir,
* justo después del sector de arranque copiado por la BIOS.
*
* El kernel almacena su tamaño en sectores en una variable que se encuentra
* definida exactamente en un desplazamiento de 4 bytes a partir su inicio.
*
* El bootsector lee el primer sector del kernel (el segundo sector de la
* imagen de disco), y de este primer sector obtiene el numero de sectores (-1)
* que debe leer para cargar el kernel completamente. Luego, carga los demas
* sectores del kernel.
*
* Con el fin de simplificar el proceso de lectura, se implementa una rutina
* que recibe como parametro el numero lineal de sector a leer, y la dirección de
* memoria (segmento:offset) en el cual se debe leer.
*
* Para cargar el kernel, se implementa la rutina 'read_lba_from_floppy',
* que oculta los detalles de la geometria de un  disco floppy.
*
* El bootsector hace uso de los servicios de la BIOS para obtener el mapa de
* memoria disponible, y lo almacena dentro del kernel en el espacio destinado
* para ello: 12 bytes a partir del inicio del kernel, es decir, en la dirección
* lineal 0x8000 + 12. En la posición 0x8000 + 8 se almacena el número de
* entradas que contiene el mapa de memoria.
*
* Finalmente, el bootsector le transfiere el control al kernel por medio
* de una instruccion ljmp.
*/

/** @verbatim */

.intel_syntax noprefix /* Usar sintaxis Intel, si prefijo para los registros */
.code16     /* 16 bits- Modo Real */

.section .text  /* Seccion de texto */

.globl start

/* Simbolo de entrada del bootsector. En este punto comienza su ejecucion. */
start:

    ljmp 0x7C0 : entry_point /* Asegurarse que cs tome el valor de 0x7C0
                                 ip toma el valor del desplazamiento
                                 de la etiqueta 'entry_point' */

/* La ejecucion continua en este punto, pero se garantiza que el valor de cs
 es 0x7C0
*/
entry_point:

	/* ax= cs */
    mov ax, cs

    /* ds = ax */
    mov ds, ax

	/* Tanto CS com DS ahora apuntan al segmento que comienza en 0x7C00*/

    /* Configurar la pila en una region de memoria
     que no este siendo utilizada. */

     /* Ver archivo real_mode_memory.txt para los detalles de como la BIOS
     configura el primer MegaByte de memoria en modo real. */

    cli     /* Deshabilitar las interrupciones */

	/* ax = 0x50 */
    mov ax, 0x50

    /* ss = ax */
    mov ss, ax     /* No se puede establecer directamente ss, usar ax*/

    /* sp = 0xB00 */
    mov sp, 0xB00  /* Configurar el tope de la pila a =0x500 + 0xB00 = 0x5B00*/

    /* Leer el primer sector del kernel */

   /* El kernel se encuentra contiguo al sector de arranque, es decir
   en el sector 2 del floppy. */

   push 0x0000
   push 0x800
   push 0x1
   /* read_lba_from_floppy(i, segmento, offset)*/
   call read_lba_from_floppy
   add sp, 6

   /* El primer sector del kernel ha sido leido en 0x8000.
   Dentro del primer sector del kernel, a un desplazamiento de 4
   bytes, se encuentra el numero de sectores. Leer ese valor
   para leer los n-1 sectores faltantes del kernel.
   */

   /* configurar es para que apunte al segmento que comienza en 0x8000,
   la dirección de memoria  en la cual se va a leer el kernel desde disco */

   /*  ax = 0x800 */
   mov ax, 0x800

   /* es = ax */
   mov es, ax

   /* es = 0x800.  es:(4) = (0x800 * 0x10) + 4 = 0x8004, la posicion
     de la variable que almacena el tamano del kernel en sectores.

     cx = [0x8004]

     */

   mov cx, es:[4]

   /* cx = cx - 1 */
   decw cx /* Ya se leyo el primer sector, restar 1 al numero de sectores*/

   /* Los sectores 2.. n del kernel se deben leer a partir de 0x8200, ya
   que el primer sector de 512 bytes (0x200), fue leido en 0x8000 */

   /* segment = 0x8200 */
   /* equivalente a mov WORD PTR [segment], (0x800 + 0x20) */
   movw [segment], (0x800 + 0x20)

   /* El siguiente sector a leer: 0=bootsector, 1 = primer sector del kernel
      Empezar en el sector LBA 2
   */

   /* lba_sector = 2 */
   /* equivalente a mov WORD PTR [lba_sector], 0x02*/
   movw [lba_sector], 0x02

read_kernel_sectors:    /* Ciclo para leer los sectores 2..n del kernel*/
   push 0x0000 /* Desplazamiento: Siempre cero */
   push [segment] /* Segmento: comienza en 0x820, dirección 0x8200*/
   push [lba_sector] /* Sector LBA a leer, comienza en 2*/
   /* Llamar a la rutina read_lba_from_floppy(i, segmento, offset=0)*/
   call read_lba_from_floppy
   add sp, 6

   /* lba_sector = lba_sector + 1 */
   incw [lba_sector] /* Incrementar el sector lba a leer */

   /* segment = segment + 0x20 */
   addw [segment], 0x20 /* Incrementar el segmento en el cual se va a leer
                          el sector */

    /* Imprimir un '+' por cada sector leido. Debido a que con este ciclo
    se leen n-1 sectores del kernel, deberia imprimir n-1 '+'*/

    mov al, '+'
    mov ah, 0x0E
    int 0x10

   loop read_kernel_sectors /* Decrementar cx, e iterar si cx > 0*/

    /* Detener el motor del floppy antes de pasar el control al kernel */
    mov dx, 0x3F2 /* Puerto de e/s del controlador del floppy */
    xor al, al
    outb dx, al

   /* Usar los servicios de la BIOS para obtener el mapa de memoria del sistema.
   http://www.uruk.org/orig-grub/mem64mb.html
   Se usa el servicio INT 0x15, 0xE820
   Se deben configurar los siguientes registros:
   EAX contiene 0xE820, la función a utilizar
   ES:DI contiene la ubicación de un buffer en memoria en el cual la BIOS
   almacenará la información
   EBX contiene el código de continuación, incialmente cero.
   EDX contiene la cadena 'SMAP' (0x534D4150), la cual le indica a la BIOS que
   se desea obtener el mapa de memoria.
   ECX contiene el tamaño máximo en bytes de la estructura que se desea recibir.
   El tamaño mínimo es 20 bytes, se usarán 24 bytes (0x18).

   En respuesta, se tiene la siguiente información:
   Carry Flag : Activa si ocurrió un error.
   EAX: Contendrá 'SMAP', lo cual significa que se almacenaron los datos en la
   dirección apuntada por ES:DI.
   ES:DI: Dirección del buffer, debe ser igual a la dirección establecida antes
   de invocar el servicio
   EBX: Valor de continuación, que debe ser enviado para las llamadas sucesivas.
   Cuando este valor sea cero, significa que se obtuvo la información de la
   última región de memoria.
   */


   /* ES:DI : Ubicación del buffer en el cual la BIOS almacena cada registro */
   mov ax, 0x800
   mov es, ax
   mov edi, 12

   /* FS = 0x800, apunta al segmento que comienza en 0x8000 */
   mov fs, ax

   /* EBX inicialmente en cero */
   xor ebx, ebx

get_memory_map:
   /* EAX = 0xE820, función a invocar */
   mov eax, 0xE820

   /* ECX  = 24, número máximo de bytes a obtener */
   mov ecx, 0x18

   /* EDX = 'SMAP' 0x534D4150 */
   mov edx, 0x534D4150

   int 0x15

   /* Si CF  = 1, ocurrió un error o se ha terminado. */
   jc end_memory_map

   /* Almacenar */
   mov es:[di + 20], cx

   add di, 24
   incw fs:[8]

   or ebx, ebx
   jz end_memory_map

   jmp get_memory_map

end_memory_map:
	

   /* Saltar a cs = 0x000, offset=0x8000 = 0x8000

	Pasa el control al codigo del kernel cargado en 0x8000
   */

   ljmp 0x0000 :  0x8000

  jmp finished /* Saltar al simbolo (la etiqueta) 'finished'

/* Simbolo (etiqueta) finished. */
finished:
   jmp finished   /* Salta a la etiqueta 'finished' (ciclo infinito) */

/* Variables del bootsector
Geometria por defecto del floppy: 80 cilindros, 2 cabezas, 18 spt.
Total sectores: 2880
Se utilizan las variables cyls, heads y spt para calcular el cilindro,
la cabeza y el sector que corresponden a un sector LBA 'i' especificado.
*/

cyls: /* Variable para almacenar los cilindros que tiene un floppy */
.word 80

heads:  /* Variable para almacenar las cabezas que tiene un floppy*/
.word 2

spt:    /* Variable para almacenar los sectores por pista de un floppy */
.word 18

segment: /* Segmento en el cual se va a leer el siguiente sector del kernel */
.word 0x0000

lba_sector: /* Numero de sector LBA del kernel a leer */
.word 0x0000

memory_map_length:
.word 0x0000

/* Implementacion de las rutinas */
/*
Rutina: read_lba_from_floppy

Descripcion: Lee un sector de un disco floppy,
por medio de interrupciones de la BIOS.
Entrada: 4(bp) : sector lba a leer (i).
         6(bp) : segmento en el cual se desea leer el sector (segmento)
         8(bp) : desplazamiento en el cual se desea leer el sector (offset)

LBA = Logical Block Addressing. Es una forma de referenciar un sector en el
disco, tomando el disco como una secuencia lineal de n sectores:
+--------------------------------------------------+
| 0  |   1  |   2  |  ...  |  i    | n-2     | n-1 |
+--------------------------------------------------+
En LBA, los sectores se enumeran de 0 a n-1, donde n es el
numero de sectores del disco.

Los discos generalmente utilizan una geometria basada en Cilindros, Sectores
y Cabezas (C/H/S).

A partir del sector LBA 'i' especificado, esta rutina calcula el sector, el
cilindro y la cabeza en la cual se encuentra el sector en un disco floppy.

*/

read_lba_from_floppy:
   push bp    /* Crear el marco de pila */
   mov bp, sp

   pusha       /* Guardar los registros de proposito general en la pila */

   push es   /* Guardar en la pila el registro es */

   /* Primer parametro: Sector (lba, comienza en 0) */
   mov ax, [bp + 4]

   /* dx = 0, dx:ax = sector LBA */
   xor dx, dx

   /* dx:ax / spt ax = cociente, dx = resto */
   divw [spt]

   /* Para la BIOS, los sectores empiezan en 1 */
   incw dx

   /* cl= dl, Numero del sector */
   mov cl, dl

   /* dx = 0 */
   xor dx, dx

   /* dx:ax = sector / spt */

   /* dx:ax (sector /spt) / cabezas ax = cociente, dx = resto */
   divw [heads]

   /* dh = cabeza */
   mov dh, dl

   /* Drive (0x0 = floppy) */
   xor dl, dl

   /* al = cilindro */
   mov ch, al

   /* Recuperar el segundo parametro de la pila: El segmento en el cual
   se debe leer el sector de disco */
   mov bx, [bp + 6]

   /* es = bx */
   mov es, bx

   /* Recuperar el tercer parametro de la pila: El desplazamiento dentro del
   segmento en el cual se debe leer el sector */
   mov bx, [bp + 8]

   /* es:bx apunta al segmento:desplazamiento en el cual se debe leer el
      sector */

read_sector:
   /* al = 0x01 */
   mov al, 0x1   /* Leer un (1) sector */

   /* ah = 0x02 */
   mov ah, 0x2   /* ah = 02h = read sectors from drive */

   int  0x13       /* Invocar los servicios de disco de la BIOS */

   jc read_sector /* Error? repetir lectura */

   pop es        /* Recuperar el valor de es de la pila */
   popa          /* Recuperar el valor de los registros de proposito general*/

   mov sp, bp    /* Cerrar el marco de pila */
   pop bp
   ret           /* Retornar de la rutina */

/*
Fin del bootsector
*/

/*
Una vez compilado, el código del bootsector no puede ocupar más de 510 bytes.
Al generar el codigo ejecutable del bootsector, el linker se encarga
de rellenar con ceros el espacio a partir de la ultima instruccion de codigo
compilado hasta completar 510 bytes, y luego incluye los bytes 0x55 y 0xAA
para generar un bootsector valido de 512 bytes.

Ver el archivo link_bootsector.ld para más detalles.

*/

/**
@endverbatim
*/
