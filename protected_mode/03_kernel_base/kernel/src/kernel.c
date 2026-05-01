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
#include <serial.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Inicializa los subsistemas base del kernel en modo protegido.
 */
void cmain(){

    setup_serial();
    serial_printf("[serial] cmain(): starting kernel initialization\n");

    /* Inicializar y limpiar la consola console.c */
    setup_console();
    serial_printf("[serial] console initialized\n");

     /* Inicializar la estructura para gestionar la memoria física. physmem.c */
    setup_physical_memory();
    serial_printf("[serial] physical memory initialized\n");

    /* Las subrutinas que se deben ejecutar ANTES de habilitar las
     * interrupciones se deben invicar en este punto */

    /* Configura la IDT y el PIC.interrupt.c */
    setup_interrupts();
    serial_printf("[serial] interrupts initialized\n");

    /* Completa la configuración de la memoria virtual. paging.c */
    setup_paging();
    serial_printf("[serial] paging initialized\n");

    /* Configurar la gestión de páginas de memoria virtual. kmem.c */
    setup_kmem();
    serial_printf("[serial] kmem initialized\n");

    /* Las subrutinas que se deben ejecutar DESPUES de habilitar las
     * interrupciones se deben invicar en este punto */

    /* Inicializar la informacion del bus PCI. pci.c  */
    setup_pci();
    serial_printf("[serial] pci initialized\n");

    /* Imprimir un mensaje por pantalla. */
    console_printf("Kernel started.\n");
    serial_printf("[serial] kernel started\n");

}
