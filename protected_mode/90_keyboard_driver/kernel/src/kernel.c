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
#include <keyboard.h>
#include <kmem.h>

void cmain(){

    char c;
    int n;
    char linea[80];

    /* Inicializar y limpiar la consola console.c*/
    console_clear();

     /* Inicializar la estructura para gestionar la memoria física. physmem.c*/
    setup_physical_memory();

    /* Configura la IDT y el PIC.interrupt.c */
    setup_interrupts();
    
    /* Completa la configuración de la memoria virtual. paging.c*/
    setup_paging();
    
    console_printf("Kernel started!\n");

    /* Detecta los dispositivos PCI conectados. pci.c */
    pci_detect();

    /* Inicializar la memoria virtual para el kernel */
    setup_kmem();

    /* Inicializar el driver de teclado */
    setup_keyboard();

    c = getchar();
    if (c == 0) {
        console_printf("TODO: implementar getchar()\n");
        for(;;);
    }
    console_printf("Caracter leido: %c\n", c);

    memset(linea, 0, 80);
    n = gets(linea, 80);

    if (n == 0) {
        console_printf("TODO: implementar gets()\n");
        for(;;);
    }
    console_printf("Buffer leido: %s\n", linea);

}
