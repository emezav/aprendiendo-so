/**
 * @file
 * @ingroup kernel_code
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License.
 * @brief Punto de entrada del kernel
 * @details
 * El kernel se carga en la dirección de memoria 0x100000 (1 MB) y empieza su
 * ejecución en la etiqueta start.
 * Este kernel cumple con la especificación multiboot, por lo cual debe
 * contener un encabezado (estructura de datos) que le indica al cargador
 * de arranque los parametros necesarios para que cargue el kernel.
 * Este kernel se almacena en una imagen de disco
 * duro, que se encuentra formateada con el sistema de archivos ext2 (linux).
 * Para cargarlo, se recurre a  un cargador de arranque (bootloader) llamado
 * GRUB.
 * GRUB es un bootloader que cumple con la Especificacion Multiboot. Puede
 * cargar cualquier kernel que cumpla con esta especificación.
 * @see http://www.gnu.org/software/grub/manual/multiboot/multiboot.html * Especificación Multiboot
*/

/** @verbatim */

/* La Especificación Multiboot requiere que el Encabezado Multiboot se encuentre
 * en los primeros 8192 bytes del archivo ejecutable, por lo cual es necesario
 * que las secciones de este archivo (start.S) se almacenen al inicio del
 * archivo ejecutable. Ver archivi link.ld */

/* Usar sintaxis Intel, si prefijo para los registros */
.intel_syntax noprefix 
.section .startcode
.code32				/* 32 bits - Modo protegido */

.global start	/* Punto de entrada del kernel */

/*
 * Definir la constante ASM para que solo se incluyan las constantes de los
 * archivos .h
 */

#define ASM 1
#include <pm.h>
#include <multiboot.h>

.global multiboot_header
/** Encabezado multiboot. Ver sección The layout of Multiboot Header  de la
 * especificación Multiboot*/
 /* El encabezado multiboot debe estar alineado a 4 bytes */
.align 4
multiboot_header:		
 /* Número mágico. Requerido. */
.long MULTIBOOT_HEADER_MAGIC
 /* Flags. Requerido. */
.long MULTIBOOT_HEADER_FLAGS
 /* Checksum. Requerido. */
.long MULTIBOOT_CHECKSUM

/* En este punto comienza la ejecución del kernel */
.global start
start:
  /* Deshabilitar las interrupciones*/
  cli

  /* Reset EFLAGS*/
  push 0
  popf

 /* Configurar la pila del kernel */
  mov esp, OFFSET kernel_stack_end

  /* Pasar el control a la rutina 'cmain' en el archivo kernel.c */
  call cmain 

  /* La función cmain() retorna a este punto. Se debe entrar en un ciclo
  infinito, para que el procesador no siga ejecutando instrucciones al finalizar
  la ejecución del kernel. */

kernel_finished:	
    hlt
	jmp kernel_finished /* Ciclo infinito */

/* Variables (globales) del kernel. Se acceden desde este código en ensamblador
 * y también podrán ser accedidas desde el código en C. */

.section .startdata
/* Pila del kernel. Por defecto se define una pila de KERNEL_STACK_SIZE. */
.align 4096
kernel_stack:
.space KERNEL_STACK_SIZE
kernel_stack_end:

/**
@endverbatim
*/
