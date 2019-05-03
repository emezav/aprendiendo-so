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
 * El kernel se almacena en una imagen de disco
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

/* Macro para generar una entrada de la Tabla de Descriptores Globales - GDT. */
/* Cada entrada ocupa exactamente 8 bytes (64 bits). Consulte la sección 3.4.5
 * Segment Descriptos del Intel Software Developers's Manual, Volumen 3A.
 *
/* gdt_entry present, base, límite, tipo, dpl, codigo_o_datos, DB (16/32 bits),
 * G (incrementos de bytes o de 4096 bytes) */
.macro gdt_entry present=0,base=0,limit=0,type=0,dpl=0,code_or_data=0,DB=0,G=0
	/* Limite 0..15 */
	.word \limit & 0x0000FFFF		
	/* Base 0..15 */
	.word \base & 0x0000FFFF  		
	/* Base 16..23 */
	.byte (\base >> 16) & 0xFF	
   /* P = 1, DPL, Code or Data , DPL,*/
	.byte ( ((\present & 1) << 7) | \
  		((\dpl & 0x03) << 5) | \
  		((\code_or_data & 1) << 4) |\
  		((\type & 0x0F))) 
	/* Limite 16 - 19, AVL = 0, L = 0, opsize (16/32 bits), G*/
	.byte ((\limit >> 16 & 0x00000F) | \
		  ((\DB & 1) << 6) | \
		  ((\G & 1) << 7) \
			)
	 .byte ( (\base >> 24) && 0xFF)
.endm

/*
* Macro: isr_no_error_code
* Este macro permite crear las  rutinas de servicio para
* interrupciones que Intel x86 que no generan codigo de error.  
* Estas son: Las excepciones Intel con vectores 0-7, 9 y 16-32 y  
* las interrupciones con vectores 33 en adelante. Con el fin de
* mantener un marco de pila constante con las rutinas de servicio
* de interrupcion que si generan codigo de error, se inserta un
* '0' como codigo de error.
*/
.macro isr_no_error_code id
 .global isr\id /* Para que esta rutina se accesible desde C*/
 isr\id:
    /* Deshabilitar las interrupciones*/
 	cli
 	/* Ahora se crea un marco de pila estandar para invocar la rutina general
 	interrupt_dispatcher. */
 	/* Codigo de error, siempre 0 para este tipo de interrupciones */
	push 0
	/* # de excepcion generada */
	push \id

	/* Saltar al código común de manejo de la interrupción */
	jmp handle_isr
.endm

/*
* Macro: isr_no_error_code
* Este macro permite crear las  rutinas de servicio de
* interrupcion (isr) para las interrupciones que si generan codigo
* de error. Estas son las excepciones Intel con vectores 8 y 10-14
*/
.macro isr_error_code id
 .global isr\id /* Para que esta rutina se accesible desde C */
 isr\id:
    /* Deshabilitar las interrupciones*/
 	cli
 	/* Ahora se crea un marco de pila estandar para invocar la rutina general
 	. */
 	/* El codigo de error es almacenado  por el procesador en la pila
 	de forma automatica cuando ocurre la excepcion
 	*/
	/* # de excepcion generada */
	push \id

	/* Saltar al código común de manejo de la interrupción */
	jmp handle_isr
.endm


/* Esta sección se incluye primero en el archivo compilado (ver link.ld) */
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
#include <paging.h>

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

.global start
start:
/* Código inicial que se encarga de mapear la ubicación física del kernel a la
* posición KERNEL_VIRT_ADDR  = KERNEL_PHYS_ADDR + KERNEL_VIRT_OFFSET 
*
* IMPORTANTE: Dado que el kernel se compila para estar ubicado en la parte alta
* de la memoria virtual (en la ubicación definida por la variable
* KERNEL_VIRT_ADDR, todas las referencias a variables son relativas a esta
* dirección. Por lo tanto en este código inicial se debe  restar
* KERNEL_VIRT_OFFSET a cualquier dirección de variable (cualquier OFFSET)
* Luego de mapear el kernel en la posición KERNEL_VIRT_ADDR y habilitar la
* memoria virtual (la paginación), todas las referencias serán
* virtuales y válidas, y podremos usarlas sin  necesidad de restarles
* KERNEL_VIRT_OFFSET.
*/

  /* Deshabilitar las interrupciones*/
  cli

  /* Guardar la ubicación de la Estructura de Información Multiboot almacenada
   * por GRUB en el registro EBX dentro de la variable multiboot_info_location */

  /* EBX = Dirección física de la Estructura de Información Multiboot */

  mov [ multiboot_info_location - KERNEL_VIRT_OFFSET ], ebx
  
  /* Configurar una pila temporal mientras se habilita la memoria virtual */
  mov esp, OFFSET kernel_stack_end - KERNEL_VIRT_OFFSET

  /* Reset EFLAGS*/
  push 0
  popf

   /* Este kernel solo puede ser cargado por un cargador de arranque compatible
   * con la especificación multiboot, por lo tanto primero se debe verificar
   * si el registro EAX contiene el número mágico almacenado por el cargador. */

   cmp eax, MULTIBOOT_BOOTLOADER_MAGIC

   /* Continuar si EAX almacena un número igual al MULTIBOOT_BOOTLOADER_MAGIC */
   je loader_is_multiboot

   /* Si se llega a este punto, no se puede continuar ya que el kernel no fue
    * cargado por un cargador de arranque compatible con Multiboot. */

   push OFFSET no_multiboot_loader - KERNEL_VIRT_OFFSET
   call kdebug
	
  /* La rutina kdebug no debería retornar, no obstante se se saca el
  * parámetro de la pila como si se hubiera retornado correctamente  y se salta
  * a kernel_kernel_finished para "bloquear" el kernel. */

   add esp, 4
   jmp kernel_finished

loader_is_multiboot:

 
  /* Ahora debemos usar la información pasada por el cargador de arranque para
   * configurar las estructuras de datos requeridas para habilitar la memoria
   * virtual (paginación). */

  /* Para activar la paginación, es necesario conocer la información memoria
   * física disponible que debe haber sido recopilada por el cargador de
   * arranque. */

/* La primera palabra de 32 bits de la Estructura de Información Multiboot
 * contiene el campo FLAGS (Ver Especificación Multiboot, Sección Boot
 * Information Format). En este campo se puede verificar si el cargador de
 * arranque proporciona el mapa de la memoria física del computador. */
  
  /* EBX = Dirección física de la Estructura de Información Multiboot */
  mov ebx, [ multiboot_info_location - KERNEL_VIRT_OFFSET ]

  /* Tomar el primer de 4 bytes (32 bits) de esa posición de memoria (FLAGS) y
   * almacenarla en eax */
  mov eax, [ebx]

  /* Verificar si el bit 6 de flags es 1, en cuyo caso el cargador proporciona
   * el mapa de la memoria física del computador. */

  /* MEMORY_MAP_PRESENT = 0x40 = 0x01000000*/
  test eax, MEMORY_MAP_PRESENT
  jnz memory_map_is_present

  /* Si el bit 6 de flags es cero, no se cuenta con el mapa de memoria física
   * por tanto no se puede continuar. */

  push OFFSET no_memory_info - KERNEL_VIRT_OFFSET
  call kdebug

  /* La rutina kdebug no debería retornar, no obstante se se saca el
  * parámetro de la pila como si se hubiera retornado correctamente  y se salta
  * a kernel_kernel_finished para "bloquear" el kernel. */
  add esp, 4
  jmp kernel_finished

memory_map_is_present:

  /* EBX = dirección física de la a Estructura de Información Multiboot. */
  mov ebx, [ multiboot_info_location - KERNEL_VIRT_OFFSET ]

  /* EAX = primer word (32 bits) de la Estructura de Información Multiboot
   * (FLAGS) */
  mov eax, [ebx]

 /* Si junto al kernel se cargaron módulos, estos se han copiado en la memoria
  * física justo después de la ubicación en la cual termina el código, los
  * datos y el BSS del kernel. Por lo tanto, es necesario calcular el número de
  * páginas adicionales que ocupan los módulos, para determinar el número de
  * entradas de tablas de página y tablas de página necesarias para mapear en
  * memoria virtual el primer MB de memoria, el kernel y los módulos
  * adicionales. */

  /* En este caso no se puede recurrir al linker, ya que los módulos son
   * compilados de forma independiente al kernel. Por tal razón, es necesario
   * usar la información de los módulos de la Estructura de Información
   * Multiboot para determinar manualmente la memoria que ocupan estos módulos
   * en memoria. */

  /* Verificar si la Estructura de Información Multiboot contiene información
   * de módulos adicionales cargados después del kernel. */

  /* Verificar si el bit 3 de flags es 1, en cuyo caso se especificaron módulos
   * que son cargados junto con el kernel. */

  /* MODULES_LOADED = 0x08 = 0x00001000*/
  test eax, MODULES_LOADED

  /* Si no se especificaron módulos, pasar directamente a calcular el número de
   * tablas de página necesarias para mapear el primer MB de memoria y el
   * kernel. */
  jz calculate_page_tables

  /* Se especificaron módulos! Calcular el tamano que ocupan y adicionarlo al
   * número de máginas que ocupa el kernel, calculado por el linker y almacenado
   * en la variable total_kernel_pages. */

  /* Número de módulos cargados, 20 bytes a partir del inicio de la Estructura
   * de Información Multiboot. */
  mov ecx, [esi + 20]
  
  /* Verificar si el conteo es cero. */
  or ecx, ecx
  jz calculate_page_tables

  /* Ubicación de la tabla de módulos, 24 bytes a partir del inicio de la
   * Estructura de Información Multibot.*/
  mov ebx, [esi + 24] 

  /* Iterar por las entradas, buscando el elemento mod_end de cada módulo (ver
   * Especificación Multiboot). Almacenar el mayor mod_end en EDX */

   /* EDX = 0 */
   xor edx, edx

read_module_info:
  mov eax, [ebx + 4]
  cmp eax, edx
  /* Pasar a la siguente entrada si el atributo mod_end es menor o igual al 
  valor almacenado en EDX*/
  jle next_module_entry

  /* Almacenar el nuevo mayor en EDX */
  mov edx, eax
next_module_entry:
  /* Avanzar 16 bytes para leer la información del siguiente módulo */
  add ebx, 16

  loop read_module_info

  /* Verificar si EDX = 0, en caso afirmativo no se encontraron módulos */
  or edx, edx
  jz calculate_page_tables

  /* La dirección de fin del módulo es física, convertirla en desplazamiento
   * relativo con respecto al inicio del kernel. */
  sub edx, KERNEL_PHYS_ADDR

  /* Se cargaron módulos. Redondear el mayor mod_end al sigiente límite de
   * página, si es necesario */
  or edx, 0xFFF
  jz no_need_to_round

  /* Los 12 bits inferiores de la dirección son diferentes de cero, hay residuo
   * al dividir entre 4096. Redondear a la siguiente página. */

  /* Borrar los 12 bits menos significativos, desplazando 12 bits a la derecha y
   * luego a la izquierda. */
  shr edx, 12
  shl edx, 12

  /* Redondear a la siguiente página */
  add edx, 0x1000

  /* Actualizar la posición del directorio de tablas de página almacenado en la
   * variable kernel_pd_addr */
   mov ebx, edx

   add ebx, KERNEL_PHYS_ADDR

   mov [kernel_pd_addr - KERNEL_VIRT_OFFSET], ebx

no_need_to_round:

  /* Dividir por 1024 el número obtenido, para convertirlo en páginas, y sumarlo
   * al número de páginas del kernel. */
  shr edx, 12

  /* Adicionar el número obtenido al número de páginas reportadas por link.ld */
  mov eax, [total_kernel_pages - KERNEL_VIRT_OFFSET]

  add eax, edx
  
  /* Y Almacenar el valor actualizado en la variable total_kernel_pages */
  mov [total_kernel_pages - KERNEL_VIRT_OFFSET ], eax

calculate_page_tables:
 /*
   Asignar el número de tablas de páginas necesario para mapear el primer MB de
   memoria y la imagen del kernel, junto con los módulos cargados,  en la parte
   baja y en la parte alta de la memoria.
  */

  /* Obtener el número de páginas reportadas por link.ld y posiblemente
   * actualizada por el código anterior, si se cargaron módulos adicionales */
  mov eax, [total_kernel_pages - KERNEL_VIRT_OFFSET]

  /* A este número de páginas se debe adicionar las páginas necesarias para
   * almacenar el directorio de tablas de página (1 página) y también las tablas
   * de página (1 página por cada tabla). */

  /* Adicionar una página para el directorio de tablas de página */
  add eax, 1

  /* Calcular el número de tablas de página*/

  /* EDX = EAX = número de páginas*/
  mov edx, eax
  /* Cada tabla de páginas puede referenciar hasta 1024 páginas, dividir el
   * número de páginas por 1024 para calcular cuantas páginas se necesita */
  shr edx, 10
  add edx, 1 /* Se necesita por lo menos una tabla de páginas */

  /* Y adicionar este total al número de páginas */
  add eax, edx

  /* Almacenar el valor actualizado en la variable del kernel */
  mov [total_kernel_pages - KERNEL_VIRT_OFFSET ], eax
  
  /* Ya se conoce cuantas páginas ocupa el kernel (y posiblemente los módulos),
   * y el número de páginas necesarias para mapear el primer MB de memoria, el
   * kernel, los módulos, el directorio de tablas de página y las tablas de
   * página. */

  /* Con este nuevo valor se debe calcular el número completo de tablas de
   * página necesarias para mapear las páginas calculadas. */

  /* Dividir por 1024 el número de páginas para obtener el número total de
   * tablas de página requeridas */
  shr eax, 10

  /* Se necesita por lo menos una tabla de páginas.*/
  add eax, 1

  mov [kernel_page_tables - KERNEL_VIRT_OFFSET], eax

  /* Ya se conoce el número de páginas y el número de tablas de página, podemos
   * mapear la memoria en el directorio y las tablas de página correspondientes
   */

  /* Primero limpiar el espacio de memoria para el directorio de tablas de
   * página*/

  /* EAX = Ubicación física del directorio de tablas de página */
  mov eax, [kernel_pd_addr - KERNEL_VIRT_OFFSET]

  /* Limpiar el directorio de tablas de páginas. */
  push eax
  call flush_page_table
  add esp, 4

  /* ECX = número de tablas de página necesarias para mapear el primer MB de
   * memoria, el kernel, los módulos adicionales, el directorio de tablas de
   * página y las tablas de página. */ 
  mov ecx, DWORD PTR [kernel_page_tables - KERNEL_VIRT_OFFSET]

 /*
   Limpiar el espacio de las tablas de página kernel.
   Las tablas de página comienzan en kernel_pd_addr + 0x1000
  */
  
 /* Obtener la ubicación del directorio de tablas de página en EAX y sumar
  * 4096 (0x1000) para ubicarnos en la siguiente página. */
  mov eax, [kernel_pd_addr - KERNEL_VIRT_OFFSET]
  add eax, 0x1000
  
flush_page_tables:
  /* Limpiar la tabla de páginas*/
  push eax
  call flush_page_table
  add esp, 4
  /* Avanzar a la siguiente tabla de páginas */
  add eax, 0x1000
  loop flush_page_tables

   /* EAX contiene la dirección física en donde termina la última tabla de
   * páginas */
   mov [kernel_initial_pagetables_end - KERNEL_VIRT_OFFSET], eax

  /* Inicializar las tablas de páginas para el primer MB y el kernel */

  /* ECX = numero de páginas del primer MB de memoria y el kernel */
   mov ecx, DWORD PTR [total_kernel_pages - KERNEL_VIRT_OFFSET]

   mov eax, [kernel_pd_addr - KERNEL_VIRT_OFFSET]
   mov edi, eax
   add edi, 0x1000

   /* Se mapean las direcciones físicas a partir de 0x000000 */
   xor eax, eax
   /* Adicionar flags 00000000000000000011 = R/W = 1, P = 1*/
   or eax, 3 

init_pagetables:
  stosd [edi], eax
  add eax, 0x1000
  loop init_pagetables

  /* Configurar las entradas en el directorio de tablas de página, de acuerdo
   * con el número de tablas de página necesarias */

  /* Al activar la paginación, se requiere que la página que se está ejecutando
   * actualmente (la que activa la paginación) se encuentre mapeada 1:1. Para
   * simplificar el proceso, se mapea 1:1 el primer MB de memoria y el kernel.*/

  /* ECX = número de tablas de página necesarias para mapear el primer MB de
   * memoria, el kernel, los módulos adicionales el directorio de tablas de
   * página y las tablas de página. */
  mov ecx, DWORD PTR [kernel_page_tables - KERNEL_VIRT_OFFSET ]
   
  /* EAX = Dirección física del directorio de tablas de página */
  mov eax, [kernel_pd_addr - KERNEL_VIRT_OFFSET]

  /* EDI = EAX */
  mov edi, eax

  /* EAX = Ubicación de la primera tabla de página: EAX + 4096 (0x1000) */
  add eax, 0x1000
  /* Adicionar flags 00000000000000000011 = R/W = 1, P = 1*/
  or eax, 3 

map_kernel_in_low_memory:
   stosd [edi], eax
   add eax, 0x1000
   loop map_kernel_in_low_memory

  /* ECX = número de tablas de página necesarias para mapear el primer MB de
   * memoria, el kernel, los módulos adicionales el directorio de tablas de
   * página y las tablas de página. */
  mov ecx, DWORD PTR [kernel_page_tables - KERNEL_VIRT_OFFSET ]
   
  /* EAX = Dirección física del directorio de tablas de página */
  mov edi, [kernel_pd_addr - KERNEL_VIRT_OFFSET]

  /* Configurar las entradas correspondientes del directorio a las tablas
   * de página que apuntan a la posición del kernel en memoria virtual */ 

   /* EBX = Desplazamiento en el espacio virtual de direcciones en el cual se
	* mapeará el primer MB de memoria, el kernel y las tablas de página. */
   mov ebx, KERNEL_VIRT_OFFSET

   /* Obtener el número de entrada correspondiente en el directorio para la
	* dirección virtual en la cual se mapeará el primer MB y el kernel */
   shr ebx, 22

   /* Y multiplicar por 4 (bytes por entrada) para encontrar el desplazamiento a
	* partir del inicio del directorio */
   shl ebx, 2

   /*Ubicar la entrada correspondiente en el directorio de tablas de página */

   /* Adicionar los bytes calculados en EBX para avanzar a la entrada
	* correspondiente en el directorio para la ubicación del kernel en la
	* memoria virtual */
   add edi, ebx

  /* Las entradas del directorio de página apuntan a las mismas tablas de pagina
   * del primer MB de memoria y el kernel */
   mov eax, [kernel_pd_addr - KERNEL_VIRT_OFFSET]

  /* EAX = Ubicación de la primera tabla de página: EAX + 4096 (0x1000) */
   add eax, 0x1000
   
   /* Adicionar flags 00000000000000000011 = R/W = 1, P = 1*/
   or eax, 3 

map_kernel_in_high_memory:
   stosd [edi], eax
   add eax, 0x1000
   loop map_kernel_in_high_memory

   /* EAX = dirección física del directorio de tablas de página */
   mov eax, [kernel_pd_addr - KERNEL_VIRT_OFFSET]

   /* Almacenar la dirección física del directorio de tablas de página en el
	* registro CR3*/
   mov cr3, eax

   /* Activar la paginación: Establecer en 1 el bit 31 de CR0 */
   /* ENABLE_PAGING = 0x80000000 */
   mov eax, cr0
   or eax, ENABLE_PAGING
   mov cr0, eax

  /* La especificación Multiboot recomienda cargar una nueva tabla global de
   * descriptores tan pronto como sea necesario. Como se habilitó la paginación,
   * es necesario cargar el registro GDTR con la dirección LINEAL (en memoria
   * virtual en este momento) en la cual se encuentra la GDT */

   /* Load GDT : Ubicación física del apuntador a la GDT */
   lgdt [kernel_gdt_pointer]

   /* LJMP permite cargar en CS el selector correspondiente al descriptor de
    * segmento de datos de 32 bits definido en nuestra GDT */
   ljmp KERNEL_CODE_SELECTOR : OFFSET after_gdt_is_loaded

/* La ejecución continúa en este punto, y el registro de segmento CS ya contiene
 * el valor 0x08. Esta entrada en la GDT define el segmento de código para el
 * kernel.*/
after_gdt_is_loaded:

  /* Cargar el selector correspondiente al descriptor del segmento de datos de
   * 32 bits definido en nuestra GDT */
  mov eax, KERNEL_DATA_SELECTOR
  mov ds, eax
  mov es, eax
  mov fs, eax
  mov gs, eax
  mov ss, eax

  /* Los selectores de los registros de código y datos ya referencian los
   * descriptores configurados en la GDT.  */

    /* Ya se encuentra activada la memoria virtual!.
    * El primer MB de memoria y el kernel se encuentran mapeados en
    * KERNEL_VIRT_ADDR.  A partir de este punto, todas las direcciones de
    * variable (OFFSET) son válidas y apuntan a la ubicación del kernel en
    * memoria virtual.
    
   /* EAX = dirección (virtual) de la etiqueta on_virtual_memory */
   lea eax, [on_virtual_memory]

   /* Saltar a la dirección en la memoria virtual!. Dado que esta dirección se
    * encuentra mapeada correctamente, no debería ocurrir fallo de página. */
   jmp eax

on_virtual_memory:

  /* El kernel ya se está ejecutando en su posición de memoria virtual!
  */

 /* Configurar la pila en la nueva posición del kernel */
  mov esp, OFFSET kernel_stack_end
  
  /* Pasar el control a la rutina 'cmain' en el archivo kernel.c */
  call cmain 

  /* La función cmain() retorna a este punto. Se debe entrar en un ciclo
  infinito, para que el procesador no siga ejecutando instrucciones al finalizar
  la ejecución del kernel. */

kernel_finished:	
    hlt
	jmp kernel_finished /* Ciclo infinito */

/* Definir las rutinas de servicio de interrupcion. Se debe tener en cuenta que
* las rutinas con vectores 0-7, 9, y 16 en adelante no generan codigo
de error, mientras que las rutinas 8, y 10-14 si generan codigo de error. */

/* Es importante recordar que las interrupciones con vector 0-31 (las primeras
 32 entradas en la IDT) corresponden a excepciones especificas de la
 arquitectura Intel. Consulte el manual de Intel Volume 3 Systems Programming
 Guide para mas detalles. */

 /* Implementacion de las 256 rutinas de servicio de interrupcion (ISR)*/

isr_no_error_code 0 /* 0: Divide By Zero*/
isr_no_error_code 1 /* 1: Debug Exception */
isr_no_error_code 2 /* 2: Non Maskable Interrupt */
isr_no_error_code 3 /* 3: Int 3 Exception */
isr_no_error_code 4 /* 4: INTO Exception */
isr_no_error_code 5 /* 5: Out of Bounds Exception */
isr_no_error_code 6 /* 6: Invalid Opcode Exception */
isr_no_error_code 7 /* 7: Coprocessor Not Available */
isr_error_code 8	/* 8: Double Fault Exception */
isr_no_error_code 9 /* 9: Coprocessor Segment Overrun Exception */
isr_error_code 10	/* 10: Bad TSS Exception */
isr_error_code 11	/* 11: Segment Not Present Exception*/
isr_error_code 12	/* 12: Stack Fault Exception*/
isr_error_code 13	/* 13: General Protection Fault Exception*/
isr_error_code 14	/* 14: Page Fault Exception*/
isr_no_error_code 15 /* 15: Reserved Exception*/
isr_no_error_code 16 /* 16: Floating Point Exception*/
isr_no_error_code 17 /* 17: Alignment Check Exception*/
isr_no_error_code 18 /* 18: Machine Check Exception*/
isr_no_error_code 19 /* 19: Reserved Exception*/
isr_no_error_code 20 /* 20: Reserved Exception*/
isr_no_error_code 21 /* 21: Reserved Exception*/
isr_no_error_code 22 /* 22: Reserved Exception*/
isr_no_error_code 23 /* 23: Reserved Exception*/
isr_no_error_code 24 /* 24: Reserved Exception*/
isr_no_error_code 25 /* 25: Reserved Exception*/
isr_no_error_code 26 /* 26: Reserved Exception*/
isr_no_error_code 27 /* 27: Reserved Exception*/
isr_no_error_code 28 /* 28: Reserved Exception*/
isr_no_error_code 29 /* 29: Reserved Exception*/
isr_no_error_code 30 /* 30: Reserved Exception*/
isr_no_error_code 31 /* 31: Reserved Exception*/

/* Las interrupciones con vector 32 en adelante no general codigo de error */

isr_no_error_code 32
isr_no_error_code 33
isr_no_error_code 34
isr_no_error_code 35
isr_no_error_code 36
isr_no_error_code 37
isr_no_error_code 38
isr_no_error_code 39
isr_no_error_code 40
isr_no_error_code 41
isr_no_error_code 42
isr_no_error_code 43
isr_no_error_code 44
isr_no_error_code 45
isr_no_error_code 46
isr_no_error_code 47
isr_no_error_code 48
isr_no_error_code 49
isr_no_error_code 50
isr_no_error_code 51
isr_no_error_code 52
isr_no_error_code 53
isr_no_error_code 54
isr_no_error_code 55
isr_no_error_code 56
isr_no_error_code 57
isr_no_error_code 58
isr_no_error_code 59
isr_no_error_code 60
isr_no_error_code 61
isr_no_error_code 62
isr_no_error_code 63
isr_no_error_code 64
isr_no_error_code 65
isr_no_error_code 66
isr_no_error_code 67
isr_no_error_code 68
isr_no_error_code 69
isr_no_error_code 70
isr_no_error_code 71
isr_no_error_code 72
isr_no_error_code 73
isr_no_error_code 74
isr_no_error_code 75
isr_no_error_code 76
isr_no_error_code 77
isr_no_error_code 78
isr_no_error_code 79
isr_no_error_code 80
isr_no_error_code 81
isr_no_error_code 82
isr_no_error_code 83
isr_no_error_code 84
isr_no_error_code 85
isr_no_error_code 86
isr_no_error_code 87
isr_no_error_code 88
isr_no_error_code 89
isr_no_error_code 90
isr_no_error_code 91
isr_no_error_code 92
isr_no_error_code 93
isr_no_error_code 94
isr_no_error_code 95
isr_no_error_code 96
isr_no_error_code 97
isr_no_error_code 98
isr_no_error_code 99
isr_no_error_code 100
isr_no_error_code 101
isr_no_error_code 102
isr_no_error_code 103
isr_no_error_code 104
isr_no_error_code 105
isr_no_error_code 106
isr_no_error_code 107
isr_no_error_code 108
isr_no_error_code 109
isr_no_error_code 110
isr_no_error_code 111
isr_no_error_code 112
isr_no_error_code 113
isr_no_error_code 114
isr_no_error_code 115
isr_no_error_code 116
isr_no_error_code 117
isr_no_error_code 118
isr_no_error_code 119
isr_no_error_code 120
isr_no_error_code 121
isr_no_error_code 122
isr_no_error_code 123
isr_no_error_code 124
isr_no_error_code 125
isr_no_error_code 126
isr_no_error_code 127
isr_no_error_code 128
isr_no_error_code 129
isr_no_error_code 130
isr_no_error_code 131
isr_no_error_code 132
isr_no_error_code 133
isr_no_error_code 134
isr_no_error_code 135
isr_no_error_code 136
isr_no_error_code 137
isr_no_error_code 138
isr_no_error_code 139
isr_no_error_code 140
isr_no_error_code 141
isr_no_error_code 142
isr_no_error_code 143
isr_no_error_code 144
isr_no_error_code 145
isr_no_error_code 146
isr_no_error_code 147
isr_no_error_code 148
isr_no_error_code 149
isr_no_error_code 150
isr_no_error_code 151
isr_no_error_code 152
isr_no_error_code 153
isr_no_error_code 154
isr_no_error_code 155
isr_no_error_code 156
isr_no_error_code 157
isr_no_error_code 158
isr_no_error_code 159
isr_no_error_code 160
isr_no_error_code 161
isr_no_error_code 162
isr_no_error_code 163
isr_no_error_code 164
isr_no_error_code 165
isr_no_error_code 166
isr_no_error_code 167
isr_no_error_code 168
isr_no_error_code 169
isr_no_error_code 170
isr_no_error_code 171
isr_no_error_code 172
isr_no_error_code 173
isr_no_error_code 174
isr_no_error_code 175
isr_no_error_code 176
isr_no_error_code 177
isr_no_error_code 178
isr_no_error_code 179
isr_no_error_code 180
isr_no_error_code 181
isr_no_error_code 182
isr_no_error_code 183
isr_no_error_code 184
isr_no_error_code 185
isr_no_error_code 186
isr_no_error_code 187
isr_no_error_code 188
isr_no_error_code 189
isr_no_error_code 190
isr_no_error_code 191
isr_no_error_code 192
isr_no_error_code 193
isr_no_error_code 194
isr_no_error_code 195
isr_no_error_code 196
isr_no_error_code 197
isr_no_error_code 198
isr_no_error_code 199
isr_no_error_code 200
isr_no_error_code 201
isr_no_error_code 202
isr_no_error_code 203
isr_no_error_code 204
isr_no_error_code 205
isr_no_error_code 206
isr_no_error_code 207
isr_no_error_code 208
isr_no_error_code 209
isr_no_error_code 210
isr_no_error_code 211
isr_no_error_code 212
isr_no_error_code 213
isr_no_error_code 214
isr_no_error_code 215
isr_no_error_code 216
isr_no_error_code 217
isr_no_error_code 218
isr_no_error_code 219
isr_no_error_code 220
isr_no_error_code 221
isr_no_error_code 222
isr_no_error_code 223
isr_no_error_code 224
isr_no_error_code 225
isr_no_error_code 226
isr_no_error_code 227
isr_no_error_code 228
isr_no_error_code 229
isr_no_error_code 230
isr_no_error_code 231
isr_no_error_code 232
isr_no_error_code 233
isr_no_error_code 234
isr_no_error_code 235
isr_no_error_code 236
isr_no_error_code 237
isr_no_error_code 238
isr_no_error_code 239
isr_no_error_code 240
isr_no_error_code 241
isr_no_error_code 242
isr_no_error_code 243
isr_no_error_code 244
isr_no_error_code 245
isr_no_error_code 246
isr_no_error_code 247
isr_no_error_code 248
isr_no_error_code 249
isr_no_error_code 250
isr_no_error_code 251
isr_no_error_code 252
isr_no_error_code 253
isr_no_error_code 254
isr_no_error_code 255

/**
 * Todas las ISR continúan su ejecución en este punto.
 * Guarda el estado del procesador en la posición actual de la pila, pasa a una
 * pila temporal del kernel e invoca la rutina de C llamada
 * interrupt_dispatcher.
 * Cuando esta rutina retorna, recupera el estado del procesador almacenado en
 * la ubicacion de current_ss:current_esp y retorna el control al punto en el
 * cual se interrumpió el procesador.
*/
handle_isr:
	/* Almacenar en la pila los registros de proposito general en el siguiente
	orden: eax, ecx, edx, ebx, esp original, ebp, esi, y edi */
	pusha
	/* Almacenar en la pila los registros de segmento de datos */
	push ds
	push es
	push fs
	push gs

	/* Este marco de pila se crea en el contexto de ejecucion actual. */

	/*
	La pila luce asi:
	+--------------------------+
	| old ss                   | Estos valores son almacenados automaticamente
	|--------------------------| en la pila cuando ocurre una excepcion
	| old esp                  | ..
	|--------------------------| ..
	| eflags                   | ..
	|--------------------------| ..
	| old cs                   | ..
	|--------------------------| ..
	| old eip                  | ..
	|--------------------------|
	| codigo de error, | 0     | Almacenado por las ISR que generan error
	|--------------------------|---------------------------------------------
	| # de interrupcion        | push \id
	|--------------------------|
	| eax                      | pusha
	|--------------------------|
	| ecx                      |(recuerde que pusha almacena en la pila los
	|--------------------------|registros en el siguiente orden:
	| edx                      |eax, ecx, edx, ebx, esp original, ebp, esi,
	|--------------------------|edi)
	| ebx                      |
	|--------------------------|
	| esp antes de pusha       |
	|--------------------------|
	| ebp                      |
	|--------------------------|
	| esi                      |
	|--------------------------|
	| edi                      |
	|--------------------------|------------------------------------------
	| ds                       | ahora los registros de segmento de datos
	|--------------------------|
	| es                       |
	|--------------------------|
	| fs                       |
	|--------------------------|
	| gs                       |
	|--------------------------|<--esp
	*/

	/* Configurar los registros de segmento de datos para que contengan
	el selector de datos para el kernel definido en la GDT */
	movw ax, KERNEL_DATA_SELECTOR
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	/* Almacenar la posicion actual del apuntador de la pila ss:esp */
	mov [current_ss], ss
	mov [current_esp], esp

	/* Apuntar al tope de la pila temporal del kernel */
	mov ss, ax
	mov esp, OFFSET interrupt_stack_top

	/* interrupt_dispatcher puede obtener un apuntador a los datos almacenados
     * en la pila mediante un apuntador a current_esp */
	call interrupt_dispatcher

	/* Retornar de la interrupcion, recuperando el estado del procesador
	a partir del marco de interrupcion almacenado. */
	jmp return_from_interrupt

/*
Rutina: return_from_interrupt
Descripcion: A partir de un marco de interrupcion, continua con la ejecucion
de una tarea.
*/
.global return_from_interrupt
return_from_interrupt:
	/* Recuperar el apuntador de la pila ss:esp almacenado luego de crear
	el marco de pila para la interrupcion */
	mov ss, [current_ss]
	mov esp, [current_esp]

	/* Ahora sacar los parametros enviados a la pila en orden inverso*/
	pop gs
	pop fs
	pop es
	pop ds
	/* los registros de proposito general */
	popa
	/* Codigo de error e interrupcion generada */
	add esp, 8

	/*
	Ahora la pila luce asi:
	+--------------------------+
	| old ss                   | Si ocurrió un cambio de contexto de pila,
	|--------------------------| se almacena la posición de la pila anterior
	| old esp                  | (SS:ESP).
	|--------------------------|
	| eflags                   | Estado del procesador (EFLAGS)
	|--------------------------|
	| old cs                   | Dirección lineal CS:EIP a la cual se debe
	|--------------------------| retornar (punto en el cual se interrumpió
	| old eip                  | el procesador)
	+--------------------------+ <-- ESP (tope de la pila)
	*/

	/* Retornar de la interrupcion */
	iret
	/* Esta rutina 'no retorna', ya que continua la ejecucion en el contexto
	que fue interrumpido. */

.globl isr_table
isr_table:
.long isr0
.long isr1
.long isr2
.long isr3
.long isr4
.long isr5
.long isr6
.long isr7
.long isr8
.long isr9
.long isr10
.long isr11
.long isr12
.long isr13
.long isr14
.long isr15
.long isr16
.long isr17
.long isr18
.long isr19
.long isr20
.long isr21
.long isr22
.long isr23
.long isr24
.long isr25
.long isr26
.long isr27
.long isr28
.long isr29
.long isr30
.long isr31
.long isr32
.long isr33
.long isr34
.long isr35
.long isr36
.long isr37
.long isr38
.long isr39
.long isr40
.long isr41
.long isr42
.long isr43
.long isr44
.long isr45
.long isr46
.long isr47
.long isr48
.long isr49
.long isr50
.long isr51
.long isr52
.long isr53
.long isr54
.long isr55
.long isr56
.long isr57
.long isr58
.long isr59
.long isr60
.long isr61
.long isr62
.long isr63
.long isr64
.long isr65
.long isr66
.long isr67
.long isr68
.long isr69
.long isr70
.long isr71
.long isr72
.long isr73
.long isr74
.long isr75
.long isr76
.long isr77
.long isr78
.long isr79
.long isr80
.long isr81
.long isr82
.long isr83
.long isr84
.long isr85
.long isr86
.long isr87
.long isr88
.long isr89
.long isr90
.long isr91
.long isr92
.long isr93
.long isr94
.long isr95
.long isr96
.long isr97
.long isr98
.long isr99
.long isr100
.long isr101
.long isr102
.long isr103
.long isr104
.long isr105
.long isr106
.long isr107
.long isr108
.long isr109
.long isr110
.long isr111
.long isr112
.long isr113
.long isr114
.long isr115
.long isr116
.long isr117
.long isr118
.long isr119
.long isr120
.long isr121
.long isr122
.long isr123
.long isr124
.long isr125
.long isr126
.long isr127
.long isr128
.long isr129
.long isr130
.long isr131
.long isr132
.long isr133
.long isr134
.long isr135
.long isr136
.long isr137
.long isr138
.long isr139
.long isr140
.long isr141
.long isr142
.long isr143
.long isr144
.long isr145
.long isr146
.long isr147
.long isr148
.long isr149
.long isr150
.long isr151
.long isr152
.long isr153
.long isr154
.long isr155
.long isr156
.long isr157
.long isr158
.long isr159
.long isr160
.long isr161
.long isr162
.long isr163
.long isr164
.long isr165
.long isr166
.long isr167
.long isr168
.long isr169
.long isr170
.long isr171
.long isr172
.long isr173
.long isr174
.long isr175
.long isr176
.long isr177
.long isr178
.long isr179
.long isr180
.long isr181
.long isr182
.long isr183
.long isr184
.long isr185
.long isr186
.long isr187
.long isr188
.long isr189
.long isr190
.long isr191
.long isr192
.long isr193
.long isr194
.long isr195
.long isr196
.long isr197
.long isr198
.long isr199
.long isr200
.long isr201
.long isr202
.long isr203
.long isr204
.long isr205
.long isr206
.long isr207
.long isr208
.long isr209
.long isr210
.long isr211
.long isr212
.long isr213
.long isr214
.long isr215
.long isr216
.long isr217
.long isr218
.long isr219
.long isr220
.long isr221
.long isr222
.long isr223
.long isr224
.long isr225
.long isr226
.long isr227
.long isr228
.long isr229
.long isr230
.long isr231
.long isr232
.long isr233
.long isr234
.long isr235
.long isr236
.long isr237
.long isr238
.long isr239
.long isr240
.long isr241
.long isr242
.long isr243
.long isr244
.long isr245
.long isr246
.long isr247
.long isr248
.long isr249
.long isr250
.long isr251
.long isr252
.long isr253
.long isr254
.long isr255

/** 
* Imprime un mensaje en la última línea de la pantalla  y bloquea el
* kernel.
* Entrada: [ebp + 8] : Dirección del mensaje a imprimir
* Esta rutina no retorna.
*/
kdebug:
  push ebp
  mov ebp, esp
  mov esi, [ebp + 8]

  xor ax, ax

  /* ah = 0x1f : Fondo azul (1), texto blanco (f) */
  mov ah, 0x1f

  xor edi, edi
  mov edi, 0xB8000
  
  /* Avanzar el cursor a la primera columna de la última línea en la pantalla */
  /* 24 líneas * 80 columnas * 2 bytes por caracter = 3840 */
  add edi, 3840

1:
  lodsb al, [esi]
  orb al, al
  jz 2f
  stosw [edi], ax
  jmp 1b

2:

  /* Detener el procesador. */
3:
   hlt
   jmp 3b

  /* Esta rutina no debería llegar a este punto! sin embargo, realizar el
   * proceso normal para  retornar de la rutina. */
  mov esp, ebp
  pop ebp
  ret

/*
* Limpia una tabla de páginas
* Entrada:
* [ebp + 8] : Dirección de la tabla a limpiar
*/
flush_page_table:
   push ebp
   mov ebp, esp

   push edi
   push eax
   push ecx

   mov edi, [ebp + 8]
   mov ecx, 1024
   xor eax, eax
   or eax, 2

   rep stosd [edi], eax

   pop ecx
   pop eax
   pop edi

   mov esp, ebp
   pop ebp
   ret

/* Variables (globales) del kernel. Se acceden desde este código en ensamblador
 * y también podrán ser accedidas desde el código en C. */
.section .startdata

/* Tabla Global de Descriptores (GDT) del kernel. */
.align 8
.global kernel_gdt
kernel_gdt:
    /* La primera entrada del gdt debe ser nula */
  //present=0,base=0,limit=0,type=0,dpl=0,code_or_data=0,DB=0,G=0
  gdt_entry 0, 0, 0, 0, 0, 0, 0, 0

  /* Descriptor de segmento de código de 32 bits para el kernel */
  gdt_entry 1, 0, 0xFFFFFFFF, CODE_SEGMENT, RING0_DPL, 1, 1, 1

  /* Descriptor de segmento de datos de 32 bits para el kernel */
  gdt_entry 1, 0, 0xFFFFFFFF, DATA_SEGMENT, RING0_DPL, 1, 1, 1

  /*Espacio para otros descriptores de segmento (TSS), código y datos para las
  * tareas, etc. */
	.space 8 * (MAX_GDT_ENTRIES - (( . - kernel_gdt) >> 3)), 0

/*Apuntador a la tabla global de descriptores, que se usa en la instruccion
LGDT.
 Debe contener un word (16 bits) con el tamano del gdt (numero de entradas * 8
 bytes /entrada) - 1, y la dirección LINEAL de 32 bits en la cual se encuentra
 la tabla de descriptores globales (GDT).
*/
kernel_gdt_pointer:
   /* Tamano de la GDT - 1: . (dirección actual) - (dirección del gdt) - 1 */
   .word . - kernel_gdt - 1 
 /* Dirección LINEAL de 32 bits en la cual se encuentra la GDT.
 La GDT se carga DESPUÉS de habilitar la paginación, por lo cual la dirección
 lineal proporcionada es la ubicación de la GDT EN MEMORIA VIRTUAL.
 */
   .long  kernel_gdt

/* Tabla de Descriptores de Interrupción (IDT) del kernel */
.align 8
.global kernel_idt
kernel_idt:
  .space 8 * MAX_IDT_ENTRIES, 0

/*Apuntador a la tabla de descriptores de interrupción, usada en la instrucción
 * LIDT.
 Debe contener un word (16 bits) con el tamano en bytes de la IDT -1 1, y la
 dirección LINEAL de 32 bits en la cual se encuentra tabla en memoria.
*/
.global kernel_idt_pointer
kernel_idt_pointer:
   /* Tamano de la IDT - 1: . (dirección actual) - (inicio de la IDT ) - 1 */
  .word . - kernel_idt - 1
 /* Dirección LINEAL de 32 bits en la cual se encuentra la IDT.
 La IDT se carga DESPUÉS de habilitar la paginación, por lo cual la dirección
 lineal proporcionada es la ubicación de la IDT EN MEMORIA VIRTUAL.
 */
  .long kernel_idt

/* Tamano del kernel en bytes, calculado por el linker (ver link.ld). */
kernel_size:
.long kernel_size_bytes

/* Número de páginas que ocupa el kernel, el directorio de tabla de páginas y
 * las tablas de página necesarias para mapear el primer MB de memoria, el
 * kernel y los módulos cargados junto con el kernel en la memoria virtual. Su
 * valor inicial lo calcula el linker, pero se actualiza adicionando el número
 * de páginas necesarias para almacenar el directorio de tablas de página y las
 * tablas de página para mapear el kernel en memoria virtual. */

total_kernel_pages:
.long tmp_total_kernel_pages

/* Número de tablas de página necesarias para mapear las páginas que ocupa el
 * primer MB de memoria, el kernel y los módulos cargados por GRUB */
 .global kernel_page_tables
kernel_page_tables:
.long 0

/* Variable que almacena la dirección física en la cual se encuentra el
 * directorio de tablas de página del kernel. Inicialmente se supone que se
 * encuentra justo al final del kernel en la dirección kernel_phys_end
 * calculada por el linker (ver link.ld). pero su valor se debe actualizar si se
 * han cargado módulos adicionales. */
.global kernel_pd_addr
kernel_pd_addr:
.long kernel_phys_end

/* Almacena el valor del selector ss en el momento en que
se va a invocar la rutina de manejo de interrupcion interrupt_handler */
.global current_ss
current_ss:
	.word 0x0000

/* Almacena el valor del registro ESP en el momento en que se
va a invocar la rutina de manejo de interrupcion interrupt_handler */
.global current_esp
current_esp:
	.long 0x00000000

/* Buffer temporal que el kernel usa como pila para invocar los manejadores
de interrupcion */

interrupt_stack_base:
/* 1024 bytes, espacio suficiente para almacenar 256 valores en la pila
 temporal*/
.space 1024

interrupt_stack_top: /* Tope de la pila temporal del kernel */
.long 0x00000000

/* Variable que almacena la dirección física en la cual se encuentra la
 * Estructura de Información Multiboot. Esta dirección física es almacenada por
 * GRUB en el registro EBX al momento de pasar el control al kernel. */
.global multiboot_info_location
multiboot_info_location:
.long 0

/* Mensaje de error */
no_multiboot_loader:
  .asciz "El cargador de arranque no es compatible con Multiboot"

/* Mensaje de error */
no_memory_info:
  .asciz "No se cuenta con el mapa de la memoria RAM"

/* Almacena la dirección física en la cual terminan las tablas de página y
 * comienza la "memoria disponible" que puede ser usada por el kernel. */
.global kernel_initial_pagetables_end
kernel_initial_pagetables_end:
.long 0

/* Pila del kernel. Por defecto se define una pila de KERNEL_STACK_SIZE. */
.align 4096
kernel_stack:
.space KERNEL_STACK_SIZE
.global kernel_stack_end
kernel_stack_end:
.long .

.space KERNEL_STACK_SIZE
.global tmp_stack
tmp_stack:
.long .


.align 4096
.global kernel_end
kernel_end:
.long .

/**
@endverbatim
*/
