/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 * @brief Identificación de dispositivos PCI
 */

#ifndef PCI_H_
#define PCI_H_

/**
 * Encabezado del espacio de configuración de un dispositivo PCI, según PCI
 * 2.3
 */
typedef struct __attribute__((packed)){
    unsigned short vendor_id;
    unsigned short device_id;
    unsigned short command;
    unsigned short status;
    unsigned char revision_id;
    unsigned char interface;
    unsigned char subclass_code;
    unsigned char class_code;
    unsigned char cache_line_size;
    unsigned char latency_timer;
    unsigned char header_type;
    unsigned char bist;
} pci_header_t;

/**
 * Estructura de los primeros 56 bytes del espacio de configuración de un
 * dispositivo PCI, según PCI 2.3.
 */
typedef struct __attribute__((packed)){
    pci_header_t header;
    unsigned int bar0;
    unsigned int bar1;
    unsigned int bar2;
    unsigned int bar3;
    unsigned int bar4;
    unsigned int bar5;
    unsigned int cardbus_cis_pointer;
    unsigned short subsystem_vendor_id;
    unsigned short subsystem_id;
    unsigned int expansion_rom_address;
    unsigned char capabilities_ponter;
    unsigned char reserved1[3];
    unsigned int reserved2;
    unsigned char interrupt_line;
    unsigned char interrupt_pin;
    unsigned char min_gnt;
    unsigned char max_lat;
    //Align to 256 bytes (PCI 2.3)
    char reserved[192];
} pci_config_t;

typedef struct {
    unsigned char bus;
    unsigned char slot;
    unsigned char function;
    pci_config_t config;
}pci_device_t;


/** @brief Referencia global a los dispositivos PCI en el sistema.*/
extern pci_device_t pci_devices[];

/**
 * @brief Realiza la detección de los dispositivos PCI.
 */
void pci_detect();

/**
 * @brief Enumera los dispositivos PCI detectados en el sistema.
 */
void pci_list();

#endif /* PCI_H_ */
