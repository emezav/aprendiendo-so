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
#include <pci.h>
#include <stdlib.h>
#include <string.h>
#include <kmem.h>
#include <kcache.h>
#include <task.h>

/**
 * @brief Primera tarea creada por el kernel.
 * @return nunca retorna.
 */
void init(void);

void cmain(){

    /** - Inicializar y limpiar la consola console.c*/
    console_clear();

     /** - Inicializar las estructuras para la memoria física. physmem.c*/
    setup_physical_memory();

    /** - Configura la IDT y el PIC.interrupt.c */
    setup_interrupts();
    
    /** - Completa la configuración de la memoria virtual. paging.c*/
    setup_paging();
    
    /** - Detecta los dispositivos PCI conectados. pci.c */
    pci_detect();

    /** - Inicializar la memoria virtual para el kernel */
    setup_kmem();

    /** - Inicializar el timer. */
    setup_timer();

    /** - Inicializar el entorno de multi tareas. */
    setup_multitasking();

    /** - Crear la tarea inicial (init). */
    create_task((unsigned int)init);

    /** - Iniciar la ejecucion de las tareas. */
    start_multitasking();

    /** - El kernel no vuelve a este punto, ya que al menos existe una 
     * tarea (init). En ultima instancia, se ejecuta idle_task (task.c) */

}

/**
 * @brief Primera tarea creada por el kernel.
 * @return nunca retorna.
 */
void init(void){
    //TODO Crear las tareas de inicio del sistema. 
    console_printf("Init started\n");
    while (1) {
        inline_assembly("hlt");
    }
}
