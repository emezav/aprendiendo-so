/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Este archivo implementa las rutinas asociadas con la gestión del TSS.
 */

#include <asm.h>
#include <console.h>
#include <pm.h>
#include <paging.h>
#include <stdlib.h>
#include <tss.h>

/** @brief Definición del segmento de estado de tarea (TSS) para el kernel. */
tss kernel_tss __attribute__((aligned(8)));

/** @brief Selector del TSS del kernel */
unsigned short kernel_tss_selector;

/**
 * @brief Esta función establece el registro de estado de tarea (TSS) para
 * el kernel.
 * */
void setup_tss(void) {

    extern unsigned int kernel_pd_addr;
	/** - Inicializar la estructura de datos Task State Segment del kernel.*/
	kernel_tss = NULL_TSS;

    kernel_tss.cr3 = kernel_pd_addr;

	/** - Buscar una entrada en la GDT para el TSS */
	kernel_tss_selector = allocate_gdt_selector();

	unsigned int base = (unsigned int)&kernel_tss;
	unsigned int limit = sizeof(tss) - 1;

	/** - Configurar la entrada en la GDT para el descriptor TSS y obtener el
	 * selector */
	setup_gdt_descriptor(kernel_tss_selector,
			base,
			limit,
			TASK_TYPE,
			RING0_DPL,
			0,
			0);

    /*
    console_printf("Kernel TSS set up at index %d\n", 
            kernel_tss_selector);
    */

	/** - Cargar el Task Register con el selector obtenido */
	/* console_printf("Selector TSS: %x\n", kernel_tss_selector); */

	inline_assembly("ltr %0"::"a"(kernel_tss_selector));

	//console_printf(" Done loading the Task Register! ");
}

