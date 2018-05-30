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
#include <ata.h>

#include <console.h>

#include <irq.h>
#include <pci.h>
#include <stdlib.h>
#include <string.h>
#include <kmem.h>
#include <kcache.h>
#include <ata.h>

void cmain(){

    char buf[512];

    /* Inicializar y limpiar la consola console.c*/
    console_clear();

     /* Inicializar la estructura para gestionar la memoria física. physmem.c*/
    setup_physical_memory();

    /* Configura la IDT y el PIC.interrupt.c */
    setup_interrupts();
    
    /* Completa la configuración de la memoria virtual. paging.c*/
    setup_paging();
    
    /* Detecta los dispositivos PCI conectados. pci.c */
    pci_detect();

    /* Inicializar la memoria virtual para el kernel. kmem.c. */
    setup_kmem();

    /* Inicializar las estructuras de datos para dispostivos ATA. ata.c.*/
    setup_ata();

    /* Suponer que el primer dispositivo ATA existe. */
    ata_device * dev = &ata_devices[0];

    if (!dev->present) {
        console_printf("ATA device not present!\n");
        return;
    }

    int res;

    res = ata_read(dev, buf, 0, 1);

    if (res == -1) {
        console_printf("Unable to read from ATA Device!\n");
    }
}

