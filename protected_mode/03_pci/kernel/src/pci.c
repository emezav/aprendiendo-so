/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 * @brief Detección de dispositivos PCI en un sistema con BIOS.
 */

#include <asm.h>
#include <console.h>
#include <pci.h>
#include <stdlib.h>
#include <string.h>

/** @brief Tabla de dispositivos PCI encontrados en el sistema */
pci_device_t pci_devices[256];

/** @brief Numero de dispositivos PCI encontrados */
int pci_device_count = 0;

/**
 * Lee un registro del espacio de configuración PCI
 * @param char bus
 * @param char slot (dispositivo)
 * @param char funcion
 * @param char offset (registro a leer)
 */
unsigned int read_pci_register(unsigned char bus, unsigned char slot, unsigned
        char function, unsigned char offset) {
    unsigned int address;
    unsigned int data;

    //Last bit of address must be set.
    address = (unsigned int)( (1 << 31) | (bus << 16 ) | (slot << 11) 
            | (function << 8) | (offset << 2));
    outl(0xCF8, address);
    data = inl(0xCFC);
    return data;
}

/**
 * @brief Realiza la detección de los dispositivos PCI.
 */
void pci_detect() {
    unsigned int data;
    int bus;
    int slot;
    int function;
    int i;

    //Se asume que el kernel se está ejecutando en un computador personal
    //con BIOS.
    unsigned short vendor_id;
    unsigned short device_id;
    unsigned int class_code;
    pci_config_t *pci_config;
    unsigned int * ptr;
    pci_device_count = 0;

    memset(pci_devices, 0, sizeof(pci_devices));

    for (bus = 0; bus < 256; bus++) {
        for (slot = 0; slot < 32; slot++) {
            for (function = 0; function < 8; function++) {
                //Read register 0 on b, d, f
                data = read_pci_register(bus, slot, function, 0);
                if (data != 0xffffffff) {
                    //bus, slot, function exists
                    pci_devices[pci_device_count].bus = bus;
                    pci_devices[pci_device_count].slot = slot;
                    pci_devices[pci_device_count].function = function;

                    //Point to the next entry on pci device table
                    pci_config = &(pci_devices[pci_device_count].config);
                    ptr = (unsigned int*)pci_config;

                    //Copy register 0
                    memcpy(ptr++, &data, sizeof(unsigned int));
                    //Read and copy registers 1 to 3
                    data = read_pci_register(bus, slot, function, 1);
                    memcpy(ptr++, &data, sizeof(unsigned int));
                    data = read_pci_register(bus, slot, function, 2);
                    memcpy(ptr++, &data, sizeof(unsigned int));
                    data = read_pci_register(bus, slot, function, 3);
                    memcpy(ptr++, &data, sizeof(unsigned int));
                    //if bits 0-6 of header type = 0, then use full template
                    if ((pci_config->header.header_type & 0x7f) == 0x00) {
                        //Get registers 4 to 15
                        for (i = 4; i < 16; i++) {
                            data = read_pci_register(bus, slot, function, i);
                            memcpy(ptr++, &data, sizeof(unsigned int));
                        }
                    }

                    //Move to next entry on pci devices table
                    pci_device_count++;
                }
            }
        }
    }
}

/**
 * @brief Enumera los dispositivos PCI detectados en el sistema.
 */
void pci_list() {
    int i;
    pci_device_t * ptr;

    console_printf("PCI device list\n");
    console_printf("===============\n");
    for (i = 0; i < pci_device_count; i++) {
        ptr = &pci_devices[i];

        console_printf("(%d, %d, %d): Vendor 0x%x, Device 0x%x, "
            "Class: 0x%x/0x%x/0x%x\n BARS: [0x%x 0x%x 0x%x 0x%x 0x%x 0x%x] INT: 0x%x\n",
            ptr->bus, ptr->slot, ptr->function, 
            ptr->config.header.vendor_id,
            ptr->config.header.device_id,
            ptr->config.header.class_code,
            ptr->config.header.subclass_code,
            ptr->config.header.interface,
            ptr->config.bar0,
            ptr->config.bar1,
            ptr->config.bar2,
            ptr->config.bar3,
            ptr->config.bar4,
            ptr->config.bar5,
            ptr->config.interrupt_line);
    }
    console_printf("===============\n");
}

