/**
 * @file
 * @ingroup kernel_code
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License.
 *
 * @brief Código de inicialización del kernel en C
 *
 * Este codigo recibe el control de start.S y continúa con la ejecución.
*/
#include <asm.h>
#include <console.h>
#include <irq.h>
#include <kmem.h>
#include <paging.h>
#include <pci.h>
#include <pm.h>
#include <physmem.h>
#include <stdlib.h>
#include <string.h>

void cmain(){

    /* Inicializar y limpiar la consola console.c */
    setup_console();

     /* Inicializar la estructura para gestionar la memoria física. physmem.c */
    setup_physical_memory();

    /* Las subrutinas que se deben ejecutar ANTES de habilitar las
     * interrupciones se deben invicar en este punto */

    /* Configura la IDT y el PIC.interrupt.c */
    setup_interrupts();

    /* Completa la configuración de la memoria virtual. paging.c */
    setup_paging();

    /* Configurar la gestión de páginas de memoria virtual. kmem.c */
    setup_kmem();

    /* Las subrutinas que se deben ejecutar DESPUES de habilitar las
     * interrupciones se deben invicar en este punto */

    /* Inicializar la informacion del bus PCI. pci.c  */
    setup_pci();

    /* Imprimir un mensaje por pantalla. */
    console_printf("Kernel started.\n");

}
