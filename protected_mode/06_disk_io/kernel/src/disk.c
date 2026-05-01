/**
 * @file
 * @ingroup kernel_code
 * @brief Capa minima de disco sobre ATA PIO para `06_disk_io`.
 */

#include <ata.h>
#include <disk.h>
#include <klog.h>
#include <monitor.h>
#include <serial.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_PARTITION_LBA 63
#define EXT2_SUPERBLOCK_OFFSET 1024
#define EXT2_SUPERBLOCK_SIZE 1024
#define EXT2_SUPERBLOCK_MAGIC 0xEF53
#define EXT2_ROOT_INODE 2
#define EXT2_NAME_MAX 255

typedef struct __attribute__((packed)) ext2_superblock {
    unsigned int inode_count;
    unsigned int block_count;
    unsigned int reserved_block_count;
    unsigned int free_block_count;
    unsigned int free_inode_count;
    unsigned int first_data_block;
    unsigned int log_block_size;
    unsigned int log_fragment_size;
    unsigned int blocks_per_group;
    unsigned int fragments_per_group;
    unsigned int inodes_per_group;
    unsigned int mount_time;
    unsigned int write_time;
    unsigned short mount_count;
    unsigned short max_mount_count;
    unsigned short magic;
    unsigned short state;
    unsigned short errors;
    unsigned short minor_revision_level;
    unsigned int lastcheck;
    unsigned int checkinterval;
    unsigned int creator_os;
    unsigned int revision_level;
    unsigned short default_reserved_uid;
    unsigned short default_reserved_gid;
    unsigned int first_inode;
    unsigned short inode_size;
} ext2_superblock;

typedef struct __attribute__((packed)) ext2_group_descriptor {
    unsigned int block_bitmap;
    unsigned int inode_bitmap;
    unsigned int inode_table;
    unsigned short free_blocks_count;
    unsigned short free_inodes_count;
    unsigned short used_dirs_count;
    unsigned short pad;
    unsigned int reserved[3];
} ext2_group_descriptor;

typedef struct __attribute__((packed)) ext2_inode {
    unsigned short mode;
    unsigned short uid;
    unsigned int size_lo;
    unsigned int atime;
    unsigned int ctime;
    unsigned int mtime;
    unsigned int dtime;
    unsigned short gid;
    unsigned short links_count;
    unsigned int blocks;
    unsigned int flags;
    unsigned int osd1;
    unsigned int block[15];
    unsigned int generation;
    unsigned int file_acl;
    unsigned int dir_acl;
    unsigned int faddr;
    unsigned char osd2[12];
} ext2_inode;

typedef struct __attribute__((packed)) ext2_dir_entry {
    unsigned int inode;
    unsigned short rec_len;
    unsigned char name_len;
    unsigned char file_type;
    char name[];
} ext2_dir_entry;

static int disk_ready;
static int disk_selected_device_index = -1;
static unsigned char disk_sector_buffer[DISK_SECTOR_SIZE];
static unsigned char ext2_superblock_buffer[EXT2_SUPERBLOCK_SIZE];
static unsigned char ext2_block_buffer[4096];
static ext2_inode ext2_inode_buffer;
static ext2_group_descriptor ext2_group_desc_buffer;
static char ext2_name_buffer[EXT2_NAME_MAX + 1];

static void disk_command_info(char * args);
static void disk_command_readlba(char * args);
static void disk_command_ext2sb(char * args);
static void disk_command_ext2root(char * args);
static void disk_command_ext2lsroot(char * args);
static void disk_dump_sector_summary(unsigned int lba,
        const unsigned char * sector);
static unsigned int disk_partition_start_lba(void);
static unsigned int disk_ext2_superblock_lba(void);
static int disk_read_ext2_superblock(ext2_superblock * out_superblock);
static unsigned int disk_ext2_block_size(const ext2_superblock * superblock);
static unsigned int disk_ext2_block_to_lba(const ext2_superblock * superblock,
        unsigned int block_number);
static int disk_read_ext2_block(const ext2_superblock * superblock,
        unsigned int block_number, void * buffer);
static int disk_read_ext2_inode(const ext2_superblock * superblock,
        const ext2_group_descriptor * group_desc, unsigned int inode_number,
        ext2_inode * out_inode);
static void disk_dump_ext2_superblock(const ext2_superblock * superblock);
static void disk_dump_ext2_group_descriptor(
        const ext2_group_descriptor * group_desc);
static void disk_dump_ext2_inode(unsigned int inode_number,
        const ext2_inode * inode);
static const char * disk_ext2_file_type_name(unsigned char file_type);
static void disk_dump_ext2_root_entries(const ext2_superblock * superblock,
        const ext2_inode * root_inode);

void setup_disk_io(void) {
    int index;

    disk_ready = 0;
    disk_selected_device_index = -1;
    memset(disk_sector_buffer, 0, sizeof(disk_sector_buffer));

    setup_ata_pio();

    for (index = 0; index < ATA_MAX_DEVICES; index++) {
        ata_device * device;

        device = ata_get_device(index);
        if (device == 0) {
            continue;
        }

        disk_selected_device_index = index;
        disk_ready = 1;
        serial_printf("[disk] selected ata device=%u model=\"%s\" sectors=%u\n",
                index, device->model, device->sector_count);
        break;
    }

    if (!disk_ready) {
        serial_printf("[disk] no readable ata device selected\n");
    }
}

void register_disk_monitor_commands(void) {
    monitor_register_command("ata", disk_command_info,
            "show the selected ata device");
    monitor_register_command("readlba", disk_command_readlba,
            "read one sector from disk, default lba 63");
    monitor_register_command("ext2sb", disk_command_ext2sb,
            "read and show the ext2 superblock");
    monitor_register_command("ext2root", disk_command_ext2root,
            "show the ext2 root inode from disk");
    monitor_register_command("ext2lsroot", disk_command_ext2lsroot,
            "list a few ext2 root directory entries");
}

int disk_is_ready(void) {
    return disk_ready;
}

unsigned int disk_get_sector_size(void) {
    return DISK_SECTOR_SIZE;
}

int disk_get_selected_device_index(void) {
    return disk_selected_device_index;
}

int disk_read_sectors(unsigned int lba, unsigned char count, void * buffer) {
    ata_device * device;

    if (!disk_ready) {
        return 0;
    }

    device = ata_get_device(disk_selected_device_index);
    if (device == 0) {
        return 0;
    }

    return ata_read28(device, lba, count, buffer);
}

static void disk_command_info(char * args) {
    ata_device * device;

    (void)args;

    if (!disk_ready) {
        klog_printf("disk: no ATA device is ready\n");
        return;
    }

    device = ata_get_device(disk_selected_device_index);
    if (device == 0) {
        klog_printf("disk: selected ATA device is no longer available\n");
        return;
    }

    klog_printf("disk: device=%u model=%s sectors=%u serial=%s\n",
            disk_selected_device_index, device->model, device->sector_count,
            device->serial);
    klog_printf("disk: revision=%s\n", device->revision);
    serial_printf("[disk] info device=%u model=\"%s\" serial=\"%s\" revision=\"%s\" sectors=%u\n",
            disk_selected_device_index, device->model, device->serial,
            device->revision, device->sector_count);
}

static void disk_command_readlba(char * args) {
    unsigned int lba;

    lba = DEFAULT_PARTITION_LBA;
    if (args != 0 && args[0] != 0) {
        lba = (unsigned int) atoi(args, 10);
    }

    if (!disk_ready) {
        klog_printf("disk: no ATA device is ready\n");
        serial_printf("[disk] readlba rejected: no ready device\n");
        return;
    }

    memset(disk_sector_buffer, 0, sizeof(disk_sector_buffer));
    if (!disk_read_sectors(lba, 1, disk_sector_buffer)) {
        klog_printf("disk: failed to read lba %u\n", lba);
        serial_printf("[disk] read failure lba=%u\n", lba);
        return;
    }

    klog_printf("disk: read lba %u successfully\n", lba);
    klog_printf("disk: read success lba=%u count=1\n", lba);
    disk_dump_sector_summary(lba, disk_sector_buffer);
}

static void disk_command_ext2sb(char * args) {
    (void)args;

    if (!disk_ready) {
        klog_printf("disk: no ATA device is ready\n");
        serial_printf("[disk] ext2sb rejected: no ready device\n");
        return;
    }

    if (!disk_read_ext2_superblock((ext2_superblock *) ext2_superblock_buffer)) {
        return;
    }

    disk_dump_ext2_superblock((const ext2_superblock *) ext2_superblock_buffer);
}

static void disk_command_ext2root(char * args) {
    ext2_superblock * superblock;
    unsigned int group_desc_block;

    (void)args;

    if (!disk_ready) {
        klog_printf("disk: no ATA device is ready\n");
        serial_printf("[disk] ext2root rejected: no ready device\n");
        return;
    }

    if (!disk_read_ext2_superblock((ext2_superblock *) ext2_superblock_buffer)) {
        return;
    }

    superblock = (ext2_superblock *) ext2_superblock_buffer;
    group_desc_block = superblock->first_data_block + 1;

    memset(ext2_block_buffer, 0, sizeof(ext2_block_buffer));
    if (!disk_read_ext2_block(superblock, group_desc_block, ext2_block_buffer)) {
        klog_printf("disk: failed to read ext2 group descriptor block %u\n",
                group_desc_block);
        serial_printf("[disk] ext2root group descriptor read failure block=%u\n",
                group_desc_block);
        return;
    }

    memcpy(&ext2_group_desc_buffer, ext2_block_buffer,
            sizeof(ext2_group_desc_buffer));
    disk_dump_ext2_group_descriptor(&ext2_group_desc_buffer);

    memset(&ext2_inode_buffer, 0, sizeof(ext2_inode_buffer));
    if (!disk_read_ext2_inode(superblock, &ext2_group_desc_buffer,
            EXT2_ROOT_INODE, &ext2_inode_buffer)) {
        klog_printf("disk: failed to read ext2 root inode\n");
        serial_printf("[disk] ext2root inode read failure inode=%u\n",
                EXT2_ROOT_INODE);
        return;
    }

    klog_printf("disk: ext2root ok inode=%u\n", EXT2_ROOT_INODE);
    disk_dump_ext2_inode(EXT2_ROOT_INODE, &ext2_inode_buffer);
}

static void disk_command_ext2lsroot(char * args) {
    ext2_superblock * superblock;
    unsigned int group_desc_block;

    (void)args;

    if (!disk_ready) {
        klog_printf("disk: no ATA device is ready\n");
        serial_printf("[disk] ext2lsroot rejected: no ready device\n");
        return;
    }

    if (!disk_read_ext2_superblock((ext2_superblock *) ext2_superblock_buffer)) {
        return;
    }

    superblock = (ext2_superblock *) ext2_superblock_buffer;
    group_desc_block = superblock->first_data_block + 1;

    memset(ext2_block_buffer, 0, sizeof(ext2_block_buffer));
    if (!disk_read_ext2_block(superblock, group_desc_block, ext2_block_buffer)) {
        klog_printf("disk: failed to read ext2 group descriptor block %u\n",
                group_desc_block);
        serial_printf("[disk] ext2lsroot group descriptor read failure block=%u\n",
                group_desc_block);
        return;
    }

    memcpy(&ext2_group_desc_buffer, ext2_block_buffer,
            sizeof(ext2_group_desc_buffer));

    memset(&ext2_inode_buffer, 0, sizeof(ext2_inode_buffer));
    if (!disk_read_ext2_inode(superblock, &ext2_group_desc_buffer,
            EXT2_ROOT_INODE, &ext2_inode_buffer)) {
        klog_printf("disk: failed to read ext2 root inode\n");
        serial_printf("[disk] ext2lsroot inode read failure inode=%u\n",
                EXT2_ROOT_INODE);
        return;
    }

    klog_printf("disk: ext2lsroot using block=%u size=%u\n",
            ext2_inode_buffer.block[0], ext2_inode_buffer.size_lo);
    disk_dump_ext2_root_entries(superblock, &ext2_inode_buffer);
}

static void disk_dump_sector_summary(unsigned int lba,
        const unsigned char * sector) {
    unsigned short signature;

    signature = sector[510] | (sector[511] << 8);
    klog_printf("disk: lba=%u first16=%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n",
            lba,
            sector[0], sector[1], sector[2], sector[3],
            sector[4], sector[5], sector[6], sector[7],
            sector[8], sector[9], sector[10], sector[11],
            sector[12], sector[13], sector[14], sector[15]);
    klog_printf("disk: lba=%u boot_signature=0x%x\n", lba, signature);
}

static unsigned int disk_partition_start_lba(void) {
    return DEFAULT_PARTITION_LBA;
}

static unsigned int disk_ext2_superblock_lba(void) {
    return disk_partition_start_lba()
            + (EXT2_SUPERBLOCK_OFFSET / DISK_SECTOR_SIZE);
}

static int disk_read_ext2_superblock(ext2_superblock * out_superblock) {
    unsigned int superblock_lba;

    superblock_lba = disk_ext2_superblock_lba();
    memset(ext2_superblock_buffer, 0, sizeof(ext2_superblock_buffer));
    if (!disk_read_sectors(superblock_lba, 2, ext2_superblock_buffer)) {
        klog_printf("disk: failed to read ext2 superblock at lba %u\n",
                superblock_lba);
        serial_printf("[disk] ext2sb read failure lba=%u count=2\n",
                superblock_lba);
        return 0;
    }

    memcpy(out_superblock, ext2_superblock_buffer, sizeof(ext2_superblock));
    if (out_superblock->magic != EXT2_SUPERBLOCK_MAGIC) {
        klog_printf("disk: ext2 superblock magic mismatch at lba %u: 0x%x\n",
                superblock_lba, out_superblock->magic);
        serial_printf("[disk] ext2sb magic mismatch lba=%u magic=0x%x\n",
                superblock_lba, out_superblock->magic);
        return 0;
    }

    klog_printf("disk: ext2 superblock found at lba %u\n", superblock_lba);
    klog_printf("disk: ext2sb ok partition_lba=%u superblock_lba=%u magic=0x%x\n",
            disk_partition_start_lba(), superblock_lba, out_superblock->magic);
    return 1;
}

static unsigned int disk_ext2_block_size(const ext2_superblock * superblock) {
    return 1024U << superblock->log_block_size;
}

static unsigned int disk_ext2_block_to_lba(const ext2_superblock * superblock,
        unsigned int block_number) {
    unsigned int block_size;
    unsigned int fs_offset;

    block_size = disk_ext2_block_size(superblock);
    fs_offset = block_number * block_size;
    return disk_partition_start_lba() + (fs_offset / DISK_SECTOR_SIZE);
}

static int disk_read_ext2_block(const ext2_superblock * superblock,
        unsigned int block_number, void * buffer) {
    unsigned int block_size;
    unsigned int sector_count;
    unsigned int lba;

    block_size = disk_ext2_block_size(superblock);
    if (block_size > sizeof(ext2_block_buffer)) {
        serial_printf("[disk] ext2 block too large size=%u\n", block_size);
        return 0;
    }

    sector_count = block_size / DISK_SECTOR_SIZE;
    lba = disk_ext2_block_to_lba(superblock, block_number);
    return disk_read_sectors(lba, (unsigned char) sector_count, buffer);
}

static int disk_read_ext2_inode(const ext2_superblock * superblock,
        const ext2_group_descriptor * group_desc, unsigned int inode_number,
        ext2_inode * out_inode) {
    unsigned int inode_size;
    unsigned int zero_based_inode;
    unsigned int inode_offset;
    unsigned int block_size;
    unsigned int block_index;
    unsigned int offset_in_block;

    if (inode_number == 0) {
        return 0;
    }

    inode_size = superblock->inode_size;
    if (inode_size == 0) {
        inode_size = 128;
    }

    zero_based_inode = inode_number - 1;
    inode_offset = zero_based_inode * inode_size;
    block_size = disk_ext2_block_size(superblock);
    block_index = inode_offset / block_size;
    offset_in_block = inode_offset % block_size;

    memset(ext2_block_buffer, 0, sizeof(ext2_block_buffer));
    if (!disk_read_ext2_block(superblock,
            group_desc->inode_table + block_index, ext2_block_buffer)) {
        return 0;
    }

    memcpy(out_inode, ext2_block_buffer + offset_in_block, sizeof(ext2_inode));
    return 1;
}

static void disk_dump_ext2_superblock(const ext2_superblock * superblock) {
    unsigned int block_size;

    block_size = 1024U << superblock->log_block_size;
    klog_printf("disk: ext2 inode_count=%u block_count=%u free_blocks=%u free_inodes=%u\n",
            superblock->inode_count, superblock->block_count,
            superblock->free_block_count, superblock->free_inode_count);
    klog_printf("disk: ext2 block_size=%u blocks_per_group=%u inodes_per_group=%u first_inode=%u inode_size=%u\n",
            block_size, superblock->blocks_per_group,
            superblock->inodes_per_group, superblock->first_inode,
            superblock->inode_size);
    klog_printf("disk: ext2 first_data_block=%u revision=%u mount_count=%u max_mount_count=%u\n",
            superblock->first_data_block, superblock->revision_level,
            superblock->mount_count, superblock->max_mount_count);
}

static void disk_dump_ext2_group_descriptor(
        const ext2_group_descriptor * group_desc) {
    klog_printf("disk: ext2 group0 block_bitmap=%u inode_bitmap=%u inode_table=%u used_dirs=%u\n",
            group_desc->block_bitmap, group_desc->inode_bitmap,
            group_desc->inode_table, group_desc->used_dirs_count);
}

static void disk_dump_ext2_inode(unsigned int inode_number,
        const ext2_inode * inode) {
    klog_printf("disk: ext2 inode=%u mode=0x%x size=%u links=%u blocks=%u flags=0x%x\n",
            inode_number, inode->mode, inode->size_lo, inode->links_count,
            inode->blocks, inode->flags);
    klog_printf("disk: ext2 inode=%u direct_blocks=%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n",
            inode_number,
            inode->block[0], inode->block[1], inode->block[2], inode->block[3],
            inode->block[4], inode->block[5], inode->block[6], inode->block[7],
            inode->block[8], inode->block[9], inode->block[10], inode->block[11]);
}

static const char * disk_ext2_file_type_name(unsigned char file_type) {
    switch (file_type) {
    case 1:
        return "file";
    case 2:
        return "dir";
    case 3:
        return "char";
    case 4:
        return "block";
    case 5:
        return "fifo";
    case 6:
        return "sock";
    case 7:
        return "symlink";
    default:
        return "unknown";
    }
}

static void disk_dump_ext2_root_entries(const ext2_superblock * superblock,
        const ext2_inode * root_inode) {
    unsigned int block_size;
    unsigned int offset;
    unsigned int entry_index;

    if (root_inode->block[0] == 0) {
        klog_printf("disk: ext2 root directory has no data block\n");
        return;
    }

    block_size = disk_ext2_block_size(superblock);
    memset(ext2_block_buffer, 0, sizeof(ext2_block_buffer));
    if (!disk_read_ext2_block(superblock, root_inode->block[0],
            ext2_block_buffer)) {
        klog_printf("disk: ext2 root directory read failure block=%u\n",
                root_inode->block[0]);
        return;
    }

    offset = 0;
    entry_index = 0;
    while (offset + 8 <= block_size && offset < root_inode->size_lo) {
        ext2_dir_entry * entry;
        unsigned int copy_len;

        entry = (ext2_dir_entry *) (ext2_block_buffer + offset);
        if (entry->rec_len == 0) {
            klog_printf("disk: ext2 root entry stopped at offset=%u due to rec_len=0\n",
                    offset);
            break;
        }

        if (entry->inode != 0) {
            copy_len = entry->name_len;
            if (copy_len > EXT2_NAME_MAX) {
                copy_len = EXT2_NAME_MAX;
            }

            memset(ext2_name_buffer, 0, sizeof(ext2_name_buffer));
            memcpy(ext2_name_buffer, entry->name, copy_len);
            ext2_name_buffer[copy_len] = 0;

            klog_printf("disk: ext2 root entry=%u inode=%u type=%s name=%s rec_len=%u\n",
                    entry_index, entry->inode,
                    disk_ext2_file_type_name(entry->file_type),
                    ext2_name_buffer, entry->rec_len);
            entry_index++;
        }

        offset += entry->rec_len;
    }
}
