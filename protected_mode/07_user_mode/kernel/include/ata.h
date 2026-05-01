/**
 * @file
 * @ingroup kernel_code
 * @brief Soporte minimo ATA PIO para lectura de sectores.
 */

#ifndef ATA_H_
#define ATA_H_

#define ATA_SECTOR_SIZE 512
#define ATA_MAX_CHANNELS 2
#define ATA_MAX_DEVICES_PER_CHANNEL 2
#define ATA_MAX_DEVICES (ATA_MAX_CHANNELS * ATA_MAX_DEVICES_PER_CHANNEL)

#define ATA_STATUS_ERR 0x01
#define ATA_STATUS_DRQ 0x08
#define ATA_STATUS_DF 0x20
#define ATA_STATUS_DRDY 0x40
#define ATA_STATUS_BSY 0x80

#define ATA_CMD_IDENTIFY 0xEC
#define ATA_CMD_READ_SECTORS 0x20

#define ATA_DRIVE_BASE 0xA0
#define ATA_DRIVE_LBA 0x40

typedef struct ata_channel {
    unsigned short io_base;
    unsigned short control_base;
} ata_channel;

typedef struct ata_device {
    int present;
    unsigned char channel_index;
    unsigned char drive_index;
    unsigned short features;
    unsigned int sector_count;
    char serial[21];
    char revision[9];
    char model[41];
} ata_device;

extern ata_channel ata_channels[ATA_MAX_CHANNELS];
extern ata_device ata_devices[ATA_MAX_DEVICES];

void setup_ata_pio(void);
int ata_get_device_count(void);
ata_device * ata_get_device(int index);
int ata_read28(ata_device * device, unsigned int lba, unsigned char count,
        void * buffer);

#endif /* ATA_H_ */
