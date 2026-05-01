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
#include <paging.h>
#include <pm.h>
#include <physmem.h>
#include <serial.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Inicializa consola, memoria fisica, interrupciones y paginacion.
 */
void cmain(){

    setup_serial();
    serial_printf("[serial] cmain(): starting kernel initialization\n");

    /* Inicializar y limpiar la consola console.c*/
    setup_console();
    serial_printf("[serial] console initialized\n");

     /* Inicializar la estructura para gestionar la memoria física
     physmem.c*/
    setup_physical_memory();
    serial_printf("[serial] physical memory initialized\n");

    /* Configura las rutinas de manejo de interrupción y carga la IDT
    interrupt.c */
    setup_interrupts();
    serial_printf("[serial] interrupts initialized\n");

    /* Llamar a la función setup_paging definida en paging.c*/
    setup_paging();
    serial_printf("[serial] paging initialized\n");

    console_printf("Kernel started!\n");
    serial_printf("[serial] kernel started\n");

}
