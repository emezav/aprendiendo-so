/**
 * @file bootsect.S
 * @ingroup bootsector_code
 * @author Erwin Meza Vega
 * @copyright MIT License
 * @brief Sector de Arranque para sistemas x86 con BIOS.
 * @description 
 * En este proyecto se muestra la definición y el uso de subrutinas.
*/

.intel_syntax noprefix 
.code16     

#include <bootsect.h>

.section .text 

.global start
start:
  /* Configurar CS para que apunte a 0x7C00 */
  ljmp 0x7C0: offset entry_point
  
entry_point:

  /* Configurar el registro de segmento DS */
  mov ax, cs
  mov ds, ax

  /* Configurar la pila */
  cli
  mov ax, 0x9000
  mov ss, ax
  mov sp, 0xFC00
  sti

  /* Leer el primer sector del kernel (sector LBA 1 del disco) a la primera
   * posición de memoria disponible en modo real: 0x500 */
  push 0x0000
  push 0x50
  push 0x1
  /* read_lba_from_floppy(1, 0x50, 0x0000) */
  call read_lba_from_floppy
  add sp, 6

  /* Modificar FS para que apunte al segmento que inicia en 0x500 */
  mov ax, 0x50
  mov fs, ax

  /* A 4 bytes dede el inicio del primer sector del kernel se encuentra el
   * número de sectores que ocupa el kernel en el disco.
   */
  mov cx, WORD PTR fs:[0x04]

  /* A 6 bytes desde el inicio del primer sector del kernel se encuentra el
   * selector de la dirección lógica en la cual se debe cargar el kernel en
   * memoria. Almacenar este selector en el registro ES.
   */
   mov ax, WORD PTR fs:[0x06]

  /** Almacenar el selector en la variable kernel_selector y en el registro
   * de segmento ES */
   mov WORD PTR [kernel_selector], ax
   mov es, ax

  /* A 8 bytes se encuentra el desplazamiento de la dirección lógica en la
   * cual se debe cargar el kernel. Almacenar este valor en DI. */
   mov di, WORD PTR fs:[0X08]

   /* Almacenar el offset en la variable kernel_offset */
   mov WORD PTR [kernel_offset], di

   /* ES:DI : Dirección lógica en la cual se debe cargar el kernel */


   /* Sector LBA a partir del cual se encuentra el kernel */
   mov WORD PTR [lba_sector], 1

  /* Ciclo para leer el kernel a la posición solicitada. */
read_kernel:

  push [kernel_offset]
  push [kernel_selector]
  push [lba_sector]
  /* read_lba_from_floppy(lba_sector, kernel_selector, kernel_offset) */
  call read_lba_from_floppy
  add sp, 6

  /* Aumentar el selector en 0x20: Mover el segmento 512 bytes más adelante */
  add WORD PTR [kernel_selector], 0x20
  /* lba_sector = lba_sector + 1 */
  inc WORD PTR [lba_sector]
  
  /* Decrementar CX y saltar a read_kernel, si CX > 0*/
  loop read_kernel

  /** Pasar el control al kernel, simulando un retorno de interrupción. Al
   * retornar de una interrupción, el procesador toma del tope de la pila
   * los valores para IP, CS y FLAGS y continúa la ejecución en ese punto.
   *
   * Entonces se debe almacenar en la pila el valor actual de FLAGS y los
   * valores de IP y CS que corresponden a la dirección lógica en la cual
   * se cargó el kerne en memoria: ES:DI. */

  pushf   /* FLAGS*/
  push es /* Selector que se almacenará en CS */
  push di /* OFFSET que se almacenará en IP */
  iret    /* Retornar de interrupción: Pasar el control al kernel! */


  /* La ejecución no debería llegar a este punto. Sin embargo, entrar una
   * espera activa si este evento se produce. 
   */

finished:
  hlt /* Detener el procesador hasta que ocurra una interrupción */
  /* Detener de nuevo el procesador. */
 jmp finished

/**
* Lee un sector de un disco floppy, haciendo uso de los servicios de disco
* de la BIOS (int 0x13).
* Entrada: 
*   [bp + 4]: sector lba a leer (i).
*   [bp + 6]: selector de la dirección lógica a la cual se lee el sector
*   [bp + 8]: desplazamiento en el cual se desea leer el sector (offset)
*
* LBA = Logical Block Addressing. Es una forma de referenciar un sector en
* el disco, tomando el disco como una secuencia lineal de n sectores:
* +----------------------------------------------------+
* |  0  |   1  |   2  |  ...  |  i    | n-2     | n-1 |
* +---------------------------------------------------+
* En LBA, los sectores se enumeran de 0 a n-1, donde n es el número de
* sectores del disco.
* 
* Los discos generalmente utilizan una geometria basada en Cilindros,
* Sectores y Cabezas (C/H/S). Esta es la geometría por defecto que usa la
* BIOS para leer y escribir en el disco.

* A partir del sector LBA 'i' especificado, esta rutina calcula el sector,
* el cilindro y la cabeza en la cual se encuentra el sector en un disco
* floppy. 
*/
read_lba_from_floppy:
  push bp    /* Crear el marco de pila */
  mov bp, sp

  pusha       /* Guardar los valores de los registros de proposito general */

  push es   /* Guardar en la pila el valor del registro ES */

  /* Primer parámetro: Sector (lba, comienza en 0) */
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

1:
  /* al = 0x01 */
  mov al, 0x1   /* Leer un (1) sector */

  /* ah = 0x02 */
  mov ah, 0x2   /* ah = 02h = read sectors from drive */

  int  0x13       /* Invocar los servicios de disco de la BIOS */

  jc 1b /* Error? repetir lectura */

  pop es        /* Recuperar el valor de es de la pila */
  popa          /* Recuperar el valor de los registros de proposito general*/

  mov sp, bp   /* Cerrar el marco de pila */
  pop bp
  ret          /* Retornar de la rutina */

/** Almacena el selector de la dirección lógica en la cual se debe cargar
 * el kernel
 */
kernel_selector:
.word 0x0000

/** Almacena el offset de la dirección lógica en la cual se debe cargar el
 * kernel
 */
kernel_offset:
.word 0x0000

/** Almacena el sector LBA del  kernel a leer */
lba_sector:
.word 0x0000

/** Número de sectores por pista de un floppy */
spt:
.word SPT

/* Número de cabezas de un floppy */
heads:
.word HEADS
