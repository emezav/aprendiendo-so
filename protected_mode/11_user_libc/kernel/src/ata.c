/**
 * @file
 * @ingroup kernel_code
 * @brief Implementacion minima ATA PIO reutilizable por `06_disk_io`.
 */

#include <asm.h>
#include <ata.h>
#include <pci.h>
#include <serial.h>
#include <string.h>

#define ATA_FEATURE_LBA28 0x0001

#define ATA_DATA_REG(channel) ((channel)->io_base + 0)
#define ATA_SECTOR_COUNT_REG(channel) ((channel)->io_base + 2)
#define ATA_LBA_LO_REG(channel) ((channel)->io_base + 3)
#define ATA_LBA_MI_REG(channel) ((channel)->io_base + 4)
#define ATA_LBA_HI_REG(channel) ((channel)->io_base + 5)
#define ATA_DRIVE_REG(channel) ((channel)->io_base + 6)
#define ATA_STATUS_REG(channel) ((channel)->io_base + 7)
#define ATA_COMMAND_REG(channel) ((channel)->io_base + 7)
#define ATA_ALT_STATUS_REG(channel) ((channel)->control_base + 0)

#define ATA_PRIMARY_IO_BASE 0x1F0
#define ATA_PRIMARY_CTRL_BASE 0x3F6
#define ATA_SECONDARY_IO_BASE 0x170
#define ATA_SECONDARY_CTRL_BASE 0x376

#define PCI_CLASS_MASS_STORAGE 0x01
#define PCI_SUBCLASS_IDE 0x01

ata_channel ata_channels[ATA_MAX_CHANNELS];
ata_device ata_devices[ATA_MAX_DEVICES];
static int ata_device_count;

static void ata_wait_400ns(ata_channel * channel);
static int ata_poll(ata_channel * channel, int need_drq);
static void ata_decode_string(char * text, int length);
static void ata_trim_string(char * text);
static void ata_apply_legacy_defaults(void);
static void ata_configure_from_pci(void);
static int ata_identify_device(unsigned char channel_index,
        unsigned char drive_index);

void setup_ata_pio(void) {
    unsigned char channel_index;
    unsigned char drive_index;

    memset(ata_channels, 0, sizeof(ata_channels));
    memset(ata_devices, 0, sizeof(ata_devices));
    ata_device_count = 0;

    ata_apply_legacy_defaults();
    ata_configure_from_pci();

    for (channel_index = 0; channel_index < ATA_MAX_CHANNELS;
            channel_index++) {
        for (drive_index = 0; drive_index < ATA_MAX_DEVICES_PER_CHANNEL;
                drive_index++) {
            if (ata_identify_device(channel_index, drive_index)) {
                ata_device_count++;
            }
        }
    }

    serial_printf("[ata] setup complete devices=%u\n", ata_device_count);
}

int ata_get_device_count(void) {
    return ata_device_count;
}

ata_device * ata_get_device(int index) {
    if (index < 0 || index >= ATA_MAX_DEVICES) {
        return 0;
    }

    if (!ata_devices[index].present) {
        return 0;
    }

    return &ata_devices[index];
}

int ata_read28(ata_device * device, unsigned int lba, unsigned char count,
        void * buffer) {
    ata_channel * channel;
    unsigned char * byte_buffer;
    int sector_index;
    unsigned char status;

    if (device == 0 || !device->present || buffer == 0) {
        return 0;
    }

    if (!(device->features & ATA_FEATURE_LBA28)) {
        serial_printf("[ata] read rejected: device lacks lba28 support\n");
        return 0;
    }

    if (count == 0) {
        serial_printf("[ata] read rejected: count=0 is not supported in this cut\n");
        return 0;
    }

    if ((lba + count) > device->sector_count) {
        serial_printf("[ata] read rejected: lba=%u count=%u sectors=%u\n", lba,
                count, device->sector_count);
        return 0;
    }

    channel = &ata_channels[device->channel_index];
    byte_buffer = (unsigned char *) buffer;

    outb(ATA_DRIVE_REG(channel), ATA_DRIVE_BASE
            | (device->drive_index << 4)
            | ATA_DRIVE_LBA
            | ((lba >> 24) & 0x0F));
    ata_wait_400ns(channel);

    outb(ATA_SECTOR_COUNT_REG(channel), count);
    outb(ATA_LBA_LO_REG(channel), lba & 0xFF);
    outb(ATA_LBA_MI_REG(channel), (lba >> 8) & 0xFF);
    outb(ATA_LBA_HI_REG(channel), (lba >> 16) & 0xFF);
    outb(ATA_COMMAND_REG(channel), ATA_CMD_READ_SECTORS);

    for (sector_index = 0; sector_index < count; sector_index++) {
        if (!ata_poll(channel, 1)) {
            return 0;
        }

        insw(ATA_DATA_REG(channel), byte_buffer, ATA_SECTOR_SIZE / 2);
        byte_buffer += ATA_SECTOR_SIZE;
    }

    status = inb(ATA_STATUS_REG(channel));
    if (status & (ATA_STATUS_ERR | ATA_STATUS_DF)) {
        serial_printf("[ata] read finished with status=0x%x\n", status);
        return 0;
    }

    return 1;
}

static void ata_wait_400ns(ata_channel * channel) {
    inb(ATA_ALT_STATUS_REG(channel));
    inb(ATA_ALT_STATUS_REG(channel));
    inb(ATA_ALT_STATUS_REG(channel));
    inb(ATA_ALT_STATUS_REG(channel));
}

static int ata_poll(ata_channel * channel, int need_drq) {
    unsigned int spins;
    unsigned char status;

    ata_wait_400ns(channel);

    for (spins = 0; spins < 1000000; spins++) {
        status = inb(ATA_STATUS_REG(channel));
        if (!(status & ATA_STATUS_BSY)) {
            if (status & (ATA_STATUS_ERR | ATA_STATUS_DF)) {
                serial_printf("[ata] poll error status=0x%x\n", status);
                return 0;
            }

            if (!need_drq || (status & ATA_STATUS_DRQ)) {
                return 1;
            }
        }
    }

    serial_printf("[ata] poll timeout status=0x%x\n", inb(ATA_STATUS_REG(channel)));
    return 0;
}

static void ata_decode_string(char * text, int length) {
    int index;
    char tmp;

    for (index = 0; index < (length - 1); index += 2) {
        tmp = text[index];
        text[index] = text[index + 1];
        text[index + 1] = tmp;
    }
}

static void ata_trim_string(char * text) {
    int index;

    index = strlen(text);
    while (index > 0) {
        if (text[index - 1] != ' ' && text[index - 1] != 0) {
            break;
        }
        text[index - 1] = 0;
        index--;
    }
}

static void ata_apply_legacy_defaults(void) {
    ata_channels[0].io_base = ATA_PRIMARY_IO_BASE;
    ata_channels[0].control_base = ATA_PRIMARY_CTRL_BASE;
    ata_channels[1].io_base = ATA_SECONDARY_IO_BASE;
    ata_channels[1].control_base = ATA_SECONDARY_CTRL_BASE;
}

static void ata_configure_from_pci(void) {
    int index;

    for (index = 0; index < pci_device_count; index++) {
        pci_device_t * device;
        unsigned short bar0;
        unsigned short bar1;
        unsigned short bar2;
        unsigned short bar3;

        device = &pci_devices[index];
        if (device->config.header.class_code != PCI_CLASS_MASS_STORAGE) {
            continue;
        }
        if (device->config.header.subclass_code != PCI_SUBCLASS_IDE) {
            continue;
        }

        bar0 = device->config.bar0 & ~(unsigned short)1;
        bar1 = device->config.bar1 & ~(unsigned short)1;
        bar2 = device->config.bar2 & ~(unsigned short)1;
        bar3 = device->config.bar3 & ~(unsigned short)1;

        if (bar0 != 0) {
            ata_channels[0].io_base = bar0;
        }
        if (bar1 != 0) {
            ata_channels[0].control_base = bar1;
        }
        if (bar2 != 0) {
            ata_channels[1].io_base = bar2;
        }
        if (bar3 != 0) {
            ata_channels[1].control_base = bar3;
        }

        serial_printf("[ata] pci ide controller bus=%u slot=%u function=%u io0=0x%x ctrl0=0x%x io1=0x%x ctrl1=0x%x\n",
                device->bus, device->slot, device->function,
                ata_channels[0].io_base, ata_channels[0].control_base,
                ata_channels[1].io_base, ata_channels[1].control_base);
        return;
    }

    serial_printf("[ata] no pci ide controller found; using legacy io ports\n");
}

static int ata_identify_device(unsigned char channel_index,
        unsigned char drive_index) {
    ata_channel * channel;
    ata_device * device;
    unsigned short identity[256];
    unsigned char status;
    unsigned char low_signature;
    unsigned char high_signature;
    unsigned int device_index;

    device_index = (channel_index * ATA_MAX_DEVICES_PER_CHANNEL) + drive_index;
    channel = &ata_channels[channel_index];
    device = &ata_devices[device_index];

    memset(identity, 0, sizeof(identity));

    outb(ATA_DRIVE_REG(channel), ATA_DRIVE_BASE | (drive_index << 4));
    ata_wait_400ns(channel);

    outb(ATA_SECTOR_COUNT_REG(channel), 0);
    outb(ATA_LBA_LO_REG(channel), 0);
    outb(ATA_LBA_MI_REG(channel), 0);
    outb(ATA_LBA_HI_REG(channel), 0);
    outb(ATA_COMMAND_REG(channel), ATA_CMD_IDENTIFY);

    status = inb(ATA_STATUS_REG(channel));
    if (status == 0) {
        return 0;
    }

    while ((status & ATA_STATUS_BSY) != 0) {
        status = inb(ATA_STATUS_REG(channel));
    }

    low_signature = inb(ATA_LBA_MI_REG(channel));
    high_signature = inb(ATA_LBA_HI_REG(channel));
    if (low_signature != 0 || high_signature != 0) {
        serial_printf("[ata] skip non-ata signature channel=%u drive=%u sig=0x%x/0x%x\n",
                channel_index, drive_index, low_signature, high_signature);
        return 0;
    }

    if (!ata_poll(channel, 1)) {
        return 0;
    }

    insw(ATA_DATA_REG(channel), identity, 256);

    memset(device, 0, sizeof(*device));
    device->present = 1;
    device->channel_index = channel_index;
    device->drive_index = drive_index;
    device->sector_count = *((unsigned int *)&identity[60]);
    memcpy(device->serial, ((char *)&identity[10]), 20);
    memcpy(device->revision, ((char *)&identity[23]), 8);
    memcpy(device->model, ((char *)&identity[27]), 40);
    device->serial[20] = 0;
    device->revision[8] = 0;
    device->model[40] = 0;
    ata_decode_string(device->serial, 20);
    ata_decode_string(device->revision, 8);
    ata_decode_string(device->model, 40);
    ata_trim_string(device->serial);
    ata_trim_string(device->revision);
    ata_trim_string(device->model);

    if (identity[49] & (1 << 9)) {
        device->features |= ATA_FEATURE_LBA28;
    }

    serial_printf("[ata] device %u channel=%u drive=%u model=\"%s\" sectors=%u lba28=%u\n",
            device_index, channel_index, drive_index, device->model,
            device->sector_count,
            (device->features & ATA_FEATURE_LBA28) ? 1 : 0);
    return 1;
}
