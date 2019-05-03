/**
 * @file
 * @ingroup kernel_code
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License.
 * @brief Punto de entrada del kernel
 * Este código pasa a modo protegido de 32 bits, y luego le pasa el control
 * al código en C del archivo kernel.c.
 */
/** @verbatim */


.intel_syntax noprefix /* Usar sintaxis Intel, si prefijo para los registros */
.code16   # 16 bits - Modo real
.section .bootcode    #Segmento de texto

/* En este archivo se define la dirección en la cual se carga el kernel */

#include <constants.h>

/* Define el simbolo de entrada necesario para el linker */
.global kernel_start
/* Punto en el cual empieza la ejecucion del kernel */
kernel_start:

/* Saltar el encabezado del kernel. Esta instrucción ocupa 3 bytes */
jmp kernel_main 

/*Encabezado del kernel: el byte 4 contiene un word con la longitud
  del kernel en sectores. */
.align 4
kernel_header:
.word kernel_sectors

/* A partir del byte 8 se almacena el número de regiones de memoria
 * encontradas*/

memory_map_entries:
.long 0

/* A partir del byte 12 se encuentra el espacio para almacenar el mapa de
 * memoria RAM disponible. Se reserva espacio para un máximo de 10 entradas de
 * 24 bytes cada una, es decir, 240 bytes. */

memory_map:
.space 240

kernel_main:

/* ax = cs */
mov ax, cs

/* ds = ax */
mov ds, ax

/* PASO A MODO PROTEGIDO DE 32 BITS */

/* Paso 1 : Deshabilitar las interrupciones */

cli

/* Paso 1.1: Habilitar la linea de direcciones A20 */

/* Esperar hasta no hayan datos para el 8042 */
1:
/* al = valor del registro de estado del 8042 */
in al, 0x64  
/* input register has data for 8042?*/
test al, 0x02 
jnz 1b

xchg bx, bx
mov al, 0xD0 /*0xd0 = Read output port: el valor del puerto
              de salida del 8042 se coloca en el registro
              de salida del controlador (0x60)*/
out 0x64, al

1:
in al, 0x64  
/* output register (0x60) has data for system?*/
test al, 0x01 
jz 1b

/* al = valor del puerto de salida */
in al, 0x60
mov ah, al /* Almacenar el valor del puerto de salida en ah */

/* Omitir activacion si A20 se encuentra habilitada  */
test al, 0x02
jnz a20_ok

or ah, 0x02 /* Activar bit 2: A20 Gate*/

mov al, 0xD1 /*0xd1 = Write output port: el byte almacenado
              en el registro de salida (0x60) se envia al puerto
              de salida del controlador*/
out 0x64, al

/*Esperar hasta que se procese el comando */
1:
in al, 0x64
test al, 2 /* input register ( 0x60 / 0x64 ) has data for 8042? */
jnz 1b  

mov al, ah /* al = bit A20 activado */
out 0x60, al /*Escribir el siguiente byte en 0x60 para habilitar A20 */

2: /* Esperar de nuevo hasta que el buffer este vacio*/
in al, 0x64
test al, 2
jnz 2b

a20_ok:

/* Paso 2:  Ejecutar LGDT para cargar un GDT valido */

lgdt [tmp_gdt_pointer]

/* Paso 3: Ejecutar una operacion MOV para activar el primer bit (PE) en
   cr0. */

mov eax, cr0
or eax, 1
mov cr0, eax

/* Paso 4: Ejecutar ljmp o call.
   El ljmp o call debe pasar a un codigo de 32 bits, que debe tener
   la directiva .code32 (prefijo de 32 bits)

   El descriptor del segmento de codigo a saltar se encuentra en la segunda
   posicion del GDT, es decir a un desplazamiento de 8 bytes (0x8) desde el
   inicio del GDT. */

ljmp 0x8 :  OFFSET in_pmode

/*  En el segmento de codigo de base 0,  saltar a la dirección lineal
    in_pmode */

/* Prefijo para codigo de 32 bits. De este punto en adelante las instrucciones
   son de 32 bits. */

.code32 /* En este punto sigue la ejecucion, en 32 bits */

in_pmode:
/*El ljmp  serializa el procesador. Esto asegura que todas las
  instrucciones que se ejecutaron en modo real se completan antes de
  pasar a modo protegido.

  Ya estamos en modo protegido!

 */

/* Paso 5: Cargar un LDT valido con la instruccion LLDT
   (EN ESTE CASO NO APLICA) */

/* Paso 6: Ejecutar una instruccion LTR para cargar el Task Register
   con un selector de segmento de la tarea inicial, o un area de memoria
   escribible que pueda ser utilizada para almacenar informacion del TSS
   en un task switch (EN ESTE CASO NO APLICA)
 */

/* Paso 7: Luego de entrar a modo protegido, los registros de segmento
   continuan con los valores que tenian en modo real. El jmp far solo cambia
   cs, por lo cual se deben actualizar los demas registros (ss, ds, es, fs y
   gs).
   Si alguno de estos registos no se va a utilizar, se deben cargar con el
   selector nulo del GDT, es decir 0x0.
 */

/*El descriptor del segmento de datos (y pila) se encuentra en la tercera
  posicion del GDT, es decir en un desplazamiento de 16 bits (0x10) desde el
  inicio del GDT. */

/* ax = 0x10 */
mov ax, 0x10

mov ds, ax   /* Todos los registros de segmento de datos apuntan al mismo */
mov es, ax   /* selector */
mov fs, ax
mov gs, ax
mov ss, ax

/* Importante: Configurar un segmento de pila temporal */
mov esp, 0x9FC00

/* Invocar a la rutina 'cmain' definida en kernel.c. */

/*
   La llamada a una rutina en 'C' sigue el mismo formato que una llamada
   a una rutina en ensamblador. Se deben pasar los parametros a la pila
   en orden inverso, luego se invoca la rutina con la instruccion call, y
   luego se recupera la pila.
 */

push OFFSET pm_message
push [kernel_size]
call cmain
add sp, 8

/* La función cmain() retorna a este punto. Se debe entrar en un ciclo
   infinito, para que el procesador no siga ejecutando instrucciones al finalizar
   la ejecución del kernel. */

loop: hlt
jmp loop /* Ciclo infinito */


/*
   TABLA GLOBAL DE DESCRIPTORES (gdt)
   Tabla de descriptores que utiliza el procesador en modo protegido de
   32 bits.
   En este caso se va a utilizar el Modelo Flat Basico, en el cual el kernel
   accede a un espacio de direcciones continuo, no segmentado. Para este modo
   se requieren al menos dos descriptores de segmento, que se mapean al
   espacio de direcciones que puede referenciar el computador (0-4GB).
   Para ello, ambos descriptores (de codigo y datos) tienen un valor base de 0
   y un limite de 1 MB. Si el bit G=1, se utilizan paginas de 4KB, por lo cual
   el limite es 1 MB * 4 KB = 4 GB.
 */

.align 8
tmp_gdt:
/* La primera entrada del gdt debe ser nula */
.word 0x0000
.word 0x0000
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00

.word 0xFFFF  /* Limite 0..15 = FFFF */
.word 0x0000  /* Base 0..15 = 0000 */
.byte 0x00    /* Base 16..23 = 00 */
.byte 0x9A    /* 10011010 P=1, DPL=0, S=1, Tipo: codigo, read/execute,
                 non conforming */
.byte 0xCF /* 11001111 G=1, D/B=1 (32 bits), L=0, AVL=0, Limite 16..19=F */
.byte 0x00 /* Base 24..31 = 00 */

.word 0xFFFF  /*Limite 0..15 = FFFF */
.word 0x0000  /*Base 0..15 = 0000 */
.byte 0x00    /*Base 16..23 = 00 */
.byte 0x92  /*10010010 P=1, DPL=0, S=1, Tipo: datos, read/write */
.byte 0xCF /*11001111  G=1, D/B=1 (32 bits) , L=0, AVL=0, Limite 16..19=F  */
.byte 0x00  /*Base 24..31 = 00  */

/*Apuntador a la tabla global de descriptores, que se usa en la instruccion
  LGDT.
  Debe contener el tamano del gdt (numero de entradas * 8 bytes /entrada) - 1,
  y la dirección lineal (en un espacio de 32 bits) en la cual se encuentra la
  tabla de descriptores globales (GDT).
 */

tmp_gdt_pointer:
.word .-tmp_gdt-1 /* Tamaño del GDT - 1: . (dirección actual) -
                     (dirección del gdt) - 1 */
.long  tmp_gdt  /* dirección lineal de 32 bits en la cual se
                   encuentra el gdt */

.data
pm_message:
.asciz "Hello from protected mode!"

kernel_byte:
.byte '*'
kernel_size:
.word kernel_sectors

.space 1024

/**
  @endverbatim
 */
