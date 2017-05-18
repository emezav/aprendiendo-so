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
#include <stdlib.h>

void cmain(){
    /* Llamar a la función setup_paging definida en paging.c*/
    setup_paging();

    /* Inicializar y limpiar la consola console.c*/
    console_clear();

     /* Inicializar la estructura para gestionar la memoria física 
     physmem.c*/
    setup_physical_memory();

    /* Configura las rutinas de manejo de interrupción y carga la IDT
    interrupt.c */
    setup_interrupts();
    
    console_printf("Kernel started!\n");

}
