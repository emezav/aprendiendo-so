/**
 * @file
 * @ingroup kernel_code
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright MIT License.
 * @brief Punto de entrada del kernel
 * Este código recibe el control del sector de arranque.
 * Para verificar que fue cargado correctamente, imprime una cadena
 * que se encuentra en su seccion de datos.
*/

/** @verbatim */

/* En este archivo se define la dirección en la cual se carga el kernel */
#include <constants.h>

.intel_syntax noprefix
.code16

/* Sección de texto del kernel */
.section .bootcode

/* Define el simbolo de entrada necesario para el linker */
.global kernel_start

/* Punto en el cual empieza la ejecucion del kernel */
kernel_start:

/* Saltar el encabezado del kernel. Esta instrucción ocupa 3 bytes */
jmp kernel_main 

/** Encabezado del kernel */
.align 4
kernel_header:
/* Número de sectores que ocupa el kernel */
.word kernel_sectors

/**
 * Dirección lógica (selector:offset) en donde el kernel debe ser
 * cargado en memoria definida en constants.h
 */
/* Selector*/
.word KERNEL_LOAD_ADDR >> 4
/* Offset */
.word KERNEL_LOAD_ADDR - ((KERNEL_LOAD_ADDR >> 4) << 4)

kernel_main:

  /* ax = cs */
  mov ax, cs

  /* ds = ax */
  mov ds, ax

  /** Configurar la pila */
  cli
  mov ax, 0x9000
  mov ss, ax
  mov sp, 0xFC00
  sti

  /** Imprimir la cadena de caracteres definida en la sección de datos del
   * kernel
   */
  push OFFSET greeting
  push ds
  call putline
  add sp, 4

  jmp kernel_finished
 
 /* Espera activa. */
kernel_finished:
  hlt /* Detener el procesador hasta que ocurra una interrupción */
  /* Detener de nuevo el procesador. */
  jmp kernel_finished    


/** 
 * Imprime una cadena de caracteres terminada en nulo.
 * Entrada:
 *  [bp + 4]: Selector de la dirección lógica de la cadena
 *  [bp + 6]: Offset de la dirección lógica de la cadena
 * Salida:
 *  Ninguno.
*/
putline:
  push bp
  mov bp, sp    /* Crear el marco de pila */
  
  push ax
  push si
  push ds

  /* DS:SI = dirección lógica de la cadena a imprimir */
  mov ax, [bp + 4]
  mov ds, ax
  mov si, [bp + 6]

1:
  /* Equivalente a:
  mov al, BYTE PTR ds:[si]
  inc si
  */
  lodsb

  /* AL = ASCII leido */
  or al, al  
  /* El caracter es nulo? terminar */
  je 2f

  /* En caso contrario, imprimir en la pantalla */

  mov ah, 0x0E
  int 0x10

  jmp 1b

2:

  pop ds    /* Recuperar los valores guardados en la pila */
  pop si      
  pop ax 

  mov sp, bp    /* Cerrar el marco de pila */
  pop bp
  ret

.section .data
greeting:
  .asciz "Hello kernel!"

/**
@endverbatim
*/
