/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Interfaz ATA
 */

#include <asm.h>
#include <ata.h>
#include <irq.h>
#include <pci.h>
#include <string.h>
#include <console.h>

/** @brief Canales ATA en el sistema. */
ata_channel ata_channels[MAX_ATA_CHANNELS]; 

/** @brief Referencia global a los dispositivos ATA */
ata_device ata_devices[MAX_ATA_DEVICES];

/** @brief Apuntador global al dispositivo actual ATA. */
ata_device * ata_current_device;

/** @brief Estado del controlador ATA al recibir la IRQ. */
char ata_status;

/** @brief Cantidad de dispositivos ATA en el sistema. */
int ata_device_count = 0;

/** @brief Bandera que indica que se ha configurado ATA. */
char ata_setup_ready = 0;

/** @brief Decodifica una cadena ATA con caracteres mezclados. */
static inline decode_ata_str(char * str, int len) {
    int i;
    char tmp;

    for (i = 0; i < len - 1; i+= 2) {
        tmp = str[i];
        str[i] = str[i + 1];
        str[i + 1] = tmp;
    }

}

/** 
 * @brief Identifica un dispositivo conectado a un canal ATA.
 * @param chan_id Canal (0 = primario, 1 = secundario).
 * @param dev_id (0 = maestro, 1 = esclavo).
 * @return 0 si se identifica correctamente el dispositivo.
*/
int ata_identify(char chan_id, char dev_id);

/** @brief Rutina de manejo de interrupción del dispositivo ATA. */
void ata_handler(interrupt_state *state) {
    unsigned char val;
    if (ata_current_device != 0) {
        ata_status = inb(ATA_STATUS_REG(ata_current_device->channel_ref));
    }else {
        //Spurious interrupt! aknowledge both status registers
        inb(ata_io_primary + 7);
        inb(ata_io_secondary + 7);
    }
}


/** @brief Configura las estructuras de datos para los dispositivos ATA.*/
void setup_ata() {
    int i;
    char chan;
    char dev;
    pci_device_t * ptr;
    extern int pci_device_count;

    ata_setup_ready = 0;
    ata_current_device = 0;

    memset(ata_channels, 0, sizeof(ata_channels));
    memset(ata_devices, 0, sizeof(ata_devices));

    ata_channels[0].channel = ata_primary;
    ata_channels[1].channel = ata_secondary;

    install_irq_handler(ATA_IRQ, ata_handler);

    if (pci_device_count == 0) {
        console_printf("No PCI devices present. Please setup PCI first.\n");
        return;
    }

    ptr = pci_devices;
    //Imprimir la lista de dispositivos PCI detectados.
    //pci_list();

    for (i = 0; i < pci_device_count; i++) {
        if (
                ptr->config.header.class_code == 0x01 &&
                (ptr->config.header.subclass_code == 0x01)
            ) {
            //Encontrado dispositivo reportado como IDE, disco duro.
            //Establecer el LSB a 0.
            unsigned short bar0 = ptr->config.bar0 & ~(unsigned short)1;
            unsigned short bar1 = ptr->config.bar1 & ~(unsigned short)1;
            unsigned short bar2 = ptr->config.bar2 & ~(unsigned short)1;
            unsigned short bar3 = ptr->config.bar3 & ~(unsigned short)1;
            unsigned short bar4 = ptr->config.bar4 & ~(unsigned short)1;

            if (bar0 == 0x00){
                bar0 = ata_io_primary;
            }

            if (bar1 == 0x00) {
                bar1 = ata_alt_primary;
            }

            if (bar2 == 0x00){
                bar2 = ata_io_secondary;
            }

            if (bar3 == 0x00) {
                bar3 = ata_alt_secondary;
            }

            ata_channels[0].io_base = bar0;
            ata_channels[0].alt_status = bar1;
            ata_channels[0].dev_ctrl = bar1;

            ata_channels[1].io_base = bar2;
            ata_channels[1].alt_status = bar3;
            ata_channels[1].dev_ctrl = bar3;
            
            //Imprimir los BAR del controlador ATA
            //console_printf("ATA 0x%x 0x%x 0x%x 0x%x\n", bar0, bar1, bar2, bar3);
            
            for (chan = 0; chan < MAX_ATA_CHANNELS; chan++) {
                for (dev = 0; dev < MAX_ATA_DEV_PER_CHANNEL; dev++) {
                    if (ata_identify(chan, dev) == 0) {
                        //Incrementar la cantidad de dispositivos
                        ata_device_count++;
                    }
                }
            }

            //Controlador ATA encontrado, terminar busqueda.
            break;
            
        }
        ptr++;
    }

    ata_setup_ready = 1;
}

/** 
 * @brief Retorna la cantidad de dispositivos ATA detectados.
 * @return Cantidad de dispositivos ATA detectados en el sistema.
 */
int ata_get_device_count() {
    return ata_device_count;
}

/** 
 * @brief Retorna una referencia al dispositivo ATA solicitado.
 * @param Indice del dispositivo 0 < MAX_ATA_DEVICES
 * @return Referencia al dispositivo, nulo si hay error.
 */
ata_device * ata_get_device(char index) {
    if (index < 0 || index >= MAX_ATA_DEVICES) {
        return 0;
    }

    return &ata_devices[index];
}


/** 
 * @brief Reinicia un canal ATA.
 * @param Referencia al canal ATA a reiniciar.
*/
void ata_reset(ata_channel * chan) {
    console_printf("TODO Reset channel %d\n", chan->channel);
}


/** 
 * @brief Identifica un dispositivo conectado a un canal ATA.
 * @param chan_id Canal (0 = primario, 1 = secundario).
 * @param dev_id (0 = maestro, 1 = esclavo).
 * @return 0 si se identifica correctamente el dispositivo.
*/
int ata_identify(char chan_id, char dev_id) {

    unsigned short buf[256];
    char dev_offset;

    memset(buf, 0, sizeof(buf));

    ata_channel * chan = &ata_channels[chan_id];

    /* Calcular la posicion (chan_id, dev_id) en el arreglo de dispositivos */
    dev_offset = (chan_id * MAX_ATA_CHANNELS) + dev_id;

    /*
    console_printf("Chan: %d Dev: %d Dev offset: %d\n", 
            chan_id,
            dev_id,
            dev_offset);
    */

    ata_device * dev = &ata_devices[dev_offset];

    chan->devices[dev_id] = dev;

    dev->id = dev_id;

    outb(ATA_SECTOR_COUNT_REG(chan), 0);
    outb(ATA_LBA_LO_REG(chan), 0);
    outb(ATA_LBA_MI_REG(chan), 0);
    outb(ATA_LBA_HI_REG(chan), 0);
    outb(ATA_DEVICE_REG(chan), (dev_id << 4));
    outb(ATA_COMMAND_REG(chan), ATA_IDENTIFY_DEVICE);

    unsigned char status = 0;

    /** - PIO para esperar respuesta del controlador. */
    do {
        status = inb(ATA_ALT_STATUS_REG(chan));
    }while (status & ATA_STATUS_BSY);

    /* 
     console_printf("Channel %d Device %d status: %b\n", 
        chan_id, 
        dev_id, 
        status); 
     */

    /** - Verificar respuesta del controlador. */
    if (status == 0 || status & ATA_STATUS_ERR) { //Status == 0 or error
        /*
         console_printf("Channel %d Device %d not present.\n", 
         chan_id,
         dev_id); 
        */
        return -1;
    }

    /* Read data! */
    insw(ATA_DATA_REG(chan), buf, 256);

    dev->present = 1;
    dev->channel_ref = chan;
    memcpy(dev->serial, (char*)&buf[10], 20);
    decode_ata_str(dev->serial, 20);
    memcpy(dev->revision, (char*)&buf[23], 8);
    decode_ata_str(dev->revision, 8);
    memcpy(dev->model, (char*)&buf[27], 40);
    decode_ata_str(dev->model, 40);
    dev->model[39] = 0;

    if (buf[49] & DMA_SUPPORTED) {
        dev->features |= DMA_SUPPORTED;
    }

    if (buf[49] & LBA_SUPPORTED) {
        dev->sectors = *(unsigned int*)&buf[60];
        dev->features |= LBA_SUPPORTED;
    }

    if (buf[83] & LBA48_SUPPORTED) {
        dev->lba48_sectors_lo = *(unsigned int*) &buf[100];
        dev->lba48_sectors_hi = *(unsigned int*) &buf[102];
        dev->features |= LBA48_SUPPORTED;
    }

    /*
    console_printf("(%d, %d) LBA-28 Sectors: %d "
            "LBA-48 lo: %d LBA-48 hi: %d Feat. %b\n", 
            chan_id, dev_id,
            dev->sectors,
            dev->lba48_sectors_lo, dev->lba48_sectors_hi,
            dev->features);
    */

    return 0;
}

/** 
 * @brief Lee hasta 256 sectores de un dispositivo ATA.
 * @param addr Direccion lineal a leer los datos
 * @param start Direccion LBA del sector inicial a leer
 * @param count Numero de sectores a leer, 0 = 256
 */
int ata_read(ata_device *dev, 
        char *addr,
        unsigned int start,
        unsigned char count) {
    
    int i;
    int n;

    if (ata_current_device != 0) {
        console_printf("Pending ATA request!\n");
        return -1;
    }

    if (!dev->present) {
        console_printf("Device not present!\n");
        return -1;
    }

    if (start >= dev->sectors) {
        console_printf("Invalid sector number\n");
        return -1;
    }

    ata_current_device = dev;
    ata_status = 0;

    ata_channel * chan = dev->channel_ref;

    outb(ATA_SECTOR_COUNT_REG(chan), count);
    outb(ATA_LBA_LO_REG(chan), (char)(start & 0x000000FF));
    outb(ATA_LBA_MI_REG(chan), (char)((start >> 8)&0x000000FF));
    outb(ATA_LBA_HI_REG(chan), (char)((start >> 16) & 0x000000FF));
    outb(ATA_DEVICE_REG(chan), 
            ((dev->id << 4) | 
             LBA_DEVICE | 
             (char)(start >> 24 & 0x0000000F)));

    outb(ATA_COMMAND_REG(chan), ATA_READ_SECTORS);

    unsigned short status;
    do {
        status = inb(ATA_ALT_STATUS_REG(chan));
    }while (status & ATA_STATUS_BSY);

    /*
     console_printf("Read Channel %d Device %d status: %b\n", 
        dev->channel_ref->channel, 
        dev->id, 
        status); 
    */
     
    n = count;
    if (n == 0) {
        n = 256;
    }

    while (n > 0) {
        //Esperar hasta que el dispositivo este listo.
        ata_status = inb(ATA_ALT_STATUS_REG(chan));
        while (ata_status & ATA_STATUS_BSY);

        if (ata_status & ATA_STATUS_ERR) {
            //console_printf("Error reading from ATA device\n");
            return -1;
        }        

        if (!(ata_status & ATA_STATUS_DRDY)) {
            //console_printf("Data not ready\n");
            return -1;
        }        

        /*
        console_printf("READ Channel %d Device %d status: %b\n", 
        dev->channel_ref->channel, 
        dev->id, 
        status); 
        */

        /* Read data! */
        insw(ATA_DATA_REG(chan), (unsigned short*)addr, 256);

        /* Loop... */
        n--;
        addr += 512;
    }

    ata_current_device = 0;
    return 0;
}

/** 
 * @brief Escribe hasta 256 sectores de un dispositivo ATA.
 * @param addr Direccion lineal en donde se encuentran los datos
 * @param start Direccion LBA del sector inicial a escribir
 * @param count Numero de sectores a escribir, 0 = 256
 */
int ata_write(ata_device *dev, 
        char *addr,
        unsigned int start,
        unsigned char count) {

    int i;
    int n;

    if (ata_current_device != 0) {
        console_printf("Pending ATA request!\n");
        return -1;
    }

    if (!dev->present) {
        console_printf("Device not present!\n");
        return -1;
    }

    if (start >= dev->sectors) {
        console_printf("Invalid sector number\n");
        return -1;
    }

    ata_current_device = dev;

    ata_channel * chan = dev->channel_ref;

    outb(ATA_SECTOR_COUNT_REG(chan), count);
    outb(ATA_LBA_LO_REG(chan), (char)(start & 0x000000FF));
    outb(ATA_LBA_MI_REG(chan), (char)((start >> 8)&0x000000FF));
    outb(ATA_LBA_HI_REG(chan), (char)((start >> 16) & 0x000000FF));
    outb(ATA_DEVICE_REG(chan), 
            ((dev->id << 4) | 
             LBA_DEVICE | 
             (char)(start >> 24 & 0x0000000F)));

    outb(ATA_COMMAND_REG(chan), ATA_WRITE_SECTORS);

    unsigned short status;
    do {
        status = inb(ATA_ALT_STATUS_REG(chan));
    }while (status & ATA_STATUS_BSY);

    /*
     console_printf("Write Channel %d Device %d status: %b\n", 
        dev->channel_ref->channel, 
        dev->id, 
        status);
    */

    n = count;
    if (n == 0) {
        n = 256;
    }
    
    while (n > 0) {
        //Esperar que el dispositivo este listo
        ata_status = inb(ATA_ALT_STATUS_REG(chan));
        while (ata_status & ATA_STATUS_BSY);

        if (ata_status & ATA_STATUS_ERR) {
            console_printf("Error writing to ATA device\n");
            return -1;
        }        

        if (!(ata_status & ATA_STATUS_DRDY)) {
            console_printf("Data to write not ready\n");
            return -1;
        }        

        /*
        console_printf("WRITE Channel %d Device %d status: %b\n", 
        dev->channel_ref->channel, 
        dev->id, 
        status); 
        */

        /* Write data! */
        outsw(ATA_DATA_REG(chan), (unsigned short*)addr, 256);

        /* Loop... */
        n--;
        addr += 512;
    }

    ata_current_device = 0;
    return 0;
}

