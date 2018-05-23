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


void dummy_task(void);
void dummy_task2(void);

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

    extern void dummy_task (void);
    extern void dummy_task2 (void);

    /*
    create_task((unsigned int)dummy_task);
    create_task((unsigned int)dummy_task2);
    */

    /** - Iniciar la ejecucion de las tareas. */
    start_multitasking();

    /** - El kernel no vuelve a este punto, ya que al menos existe el
     * proceso inactivo (idle_task : task.c). */

}

void dummy_task(void) {
    unsigned int i;
    bochs_break();
    console_printf("Task %d started\n", current_task->pid);
    console_printf("Stack at: 0x%x 0x%x -> 0x%x\n", current_task->current_esp, 
            current_task->kernel_stack,
            current_task->kernel_stack - TASK_STACK_SIZE);
    for (i = 0; i < 0xFFFF; i++) {
        unsigned char status = inb(0x64);
    }

    console_printf("Task %d finished\n", current_task->pid);
}

void dummy_task2(void) {
    unsigned int i;
    bochs_break();
    console_printf("Task %d started\n", current_task->pid);
    console_printf("Stack at: 0x%x 0x%x -> 0x%x\n", current_task->current_esp, 
            current_task->kernel_stack,
            current_task->kernel_stack - TASK_STACK_SIZE);
    for (i = 0; i < 0xFFFF; i++) {
        unsigned char status = inb(0x64);
    }
    console_printf("Task %d finished\n", current_task->pid);
}


