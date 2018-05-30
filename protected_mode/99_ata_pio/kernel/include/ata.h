/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Manejo de la interfaz ATA
 */

#ifndef ATA_H_
#define ATA_H_

/** @brief IRQ del controlador ATA */
#define ATA_IRQ 0x0E

/** @brief Numero maximo de canales ATA en el sistema */
#define MAX_ATA_CHANNELS 2

/** @brief Numero maximo de dispositivos por canal ATA */
#define MAX_ATA_DEV_PER_CHANNEL 2

/** @brief Numero maximo de dispositivos ATA en el sistema */
#define MAX_ATA_DEVICES (MAX_ATA_CHANNELS * MAX_ATA_DEV_PER_CHANNEL)

/** @brief Canales ATA */
typedef enum {
    ata_primary = 0,
    ata_secondary = 1
}ata_channel_t;

struct ata_channel;

/** @Dispositivo ATA. */
typedef struct {
    char present;
    struct ata_channel * channel_ref;
    char id;
    char serial[24];
    char revision[26];
    char model[48];
    unsigned short features;
    unsigned int sectors;
    unsigned int lba48_sectors_lo;
    unsigned int lba48_sectors_hi;
}ata_device;

/** @brief Canal ATA, agrupa hasta dos dispositivos. */
typedef struct ata_channel{
    ata_channel_t channel;
    char selected_device;
    unsigned short io_base;
    unsigned short alt_status;
    unsigned short dev_ctrl;
    unsigned short bmide_base;
    ata_device * devices[2];
}ata_channel;

/** @brief Referencia global a los canales ATA */
extern ata_channel ata_channels[];

/** @brief Referencia global a los dispositivos ATA */
extern ata_device ata_devices[];

/** @brief Cantidad de dispositivos ATA en el sistema. */
extern int ata_device_count;

/** @brief Direccion de E/S del registro de datos */
#define ATA_DATA_REG(channel) (channel->io_base)

/** @brief Direccion de E/S del registro de error */
#define ATA_ERROR_REG(channel) (channel->io_base + 1)

/** @brief Direccion de E/S del registro de carateristicas */
#define ATA_FEATURES_REG(channel) (channel->io_base + 1)

/** @brief Direccion de E/S del registro de numero de sectores */
#define ATA_SECTOR_COUNT_REG(channel) (channel->io_base + 2)

/** @brief Direccion de E/S de los bits 0-7 del sector LBA */
#define ATA_LBA_LO_REG(channel) (channel->io_base + 3)

/** @brief Direccion de E/S de los bits 8-15 del sector LBA */
#define ATA_LBA_MI_REG(channel) (channel->io_base + 4)

/** @brief Direccion de E/S de los bits 16-23 del sector LBA */
#define ATA_LBA_HI_REG(channel) (channel->io_base + 5)

/** @brief Direccion de E/S del selector del dispositivo  */
#define ATA_DEVICE_REG(channel) (channel->io_base + 6)

/** @brief Direccion de E/S del registro de estado. 
 * @description Leer de este puerto limpia cualquier IRQ pendiente. */
#define ATA_STATUS_REG(channel) (channel->io_base + 7)

/** @brief Direccion de E/S del registro de comandos.  */
#define ATA_COMMAND_REG(channel) (channel->io_base + 7)

/** @brief Direccion de E/S del registro alterno de estado. 
 * @description Leer de este puerto limpia cualquier IRQ pendiente. */
#define ATA_ALT_STATUS_REG(channel) (channel->alt_status)

/* Comandos ATA */
#define ATA_IDENTIFY_DEVICE 0xEC
#define ATA_READ_SECTORS 0x20
#define ATA_WRITE_SECTORS 0x30
#define ATA_READ_DMA 0xC8

/* Codigos de BITS del estado */
#define ATA_STATUS_BSY 0x80
#define ATA_STATUS_DRDY 0x40
#define ATA_STATUS_DRQ 0x08
#define ATA_STATUS_DRQ 0x08
#define ATA_STATUS_ERR 0x01

/* Bit siempre activo en el selector del dispositivo */
#define ATA_DRIVE 0xA0

#define LBA_DEVICE (1<<6)

/* Codigos de bits de identify device */

/** @brief bit 8 de identify device [49]*/
#define DMA_SUPPORTED (1<<8)

/** @brief bit 9 de identify device [49]*/
#define LBA_SUPPORTED (1<<9)

/** @brief bit 10 de identify device [83]*/
#define LBA48_SUPPORTED (1<<10)

typedef enum {
    ata_io_primary      = 0x1F0,
    ata_io_secondary    = 0x170
}ata_io_port;

typedef enum {
    ata_alt_primary     =  0x3F6,
    ata_alt_secondary   = 0x376
}ata_alt_status;

/** @brief Configura las estructuras de datos para los dispositivos ATA.*/
void setup_ata(void);

/** 
 * @brief Retorna la cantidad de dispositivos ATA detectados.
 * @return Cantidad de dispositivos ATA detectados en el sistema.
 */
int ata_get_device_count();

/** 
 * @brief Retorna una referencia al dispositivo ATA solicitado.
 * @param Indice del dispositivo 0 < MAX_ATA_DEVICES
 * @return Referencia al dispositivo, nulo si hay error.
 */
ata_device * ata_get_device(char index);

/** 
 * @brief Reinicia un canal ATA.
 * @param Referencia al canal ATA a reiniciar.
*/
void ata_reset(ata_channel * chan);

/** 
 * @brief Lee hasta 256 sectores de un dispositivo ATA.
 * @param addr Direccion lineal a leer los datos
 * @param start Direccion LBA del sector inicial a leer
 * @param count Numero de sectores a leer, 0 = 256
 */
int ata_read(ata_device *dev, 
        char *addr,
        unsigned int start,
        unsigned char count);

/** 
 * @brief Escribe hasta 256 sectores de un dispositivo ATA.
 * @param addr Direccion lineal en donde se encuentran los datos
 * @param start Direccion LBA del sector inicial a escribir
 * @param count Numero de sectores a escribir, 0 = 256
 */
int ata_write(ata_device *dev, 
        char *addr,
        unsigned int start,
        unsigned char count);

#endif /* ATA_H_ */
