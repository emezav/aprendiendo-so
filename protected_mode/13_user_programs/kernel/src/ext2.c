/**
 * @file
 * @ingroup kernel_code
 * @brief Lector minimo de ext2 para resolver rutas absolutas y leer archivos.
 */

#include <disk.h>
#include <ext2.h>
#include <klog.h>
#include <monitor.h>
#include <serial.h>
#include <string.h>

#define EXT2_SUPERBLOCK_OFFSET 1024
#define EXT2_SUPERBLOCK_SIZE 1024
#define EXT2_SUPERBLOCK_MAGIC 0xEF53
#define EXT2_ROOT_INODE 2
#define EXT2_NAME_MAX 255
#define EXT2_MODE_DIR 0x4000
#define EXT2_MODE_FILE 0x8000
#define EXT2_DIR_ENTRY_HEADER_SIZE 8
#define EXT2_DEFAULT_CAT_PATH "/boot/grub/menu.lst"
#define EXT2_DEFAULT_LS_PATH "/"

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

static int ext2_ready;
static ext2_superblock ext2_superblock_cache;
static unsigned char ext2_block_buffer[4096];
static unsigned char ext2_read_buffer[EXT2_FILE_READ_MAX + 1];
static char ext2_name_buffer[EXT2_NAME_MAX + 1];
static char ext2_component_buffer[EXT2_NAME_MAX + 1];
static char ext2_path_buffer[EXT2_PATH_MAX + 1];

static void ext2_command_ls(char * args);
static void ext2_command_cat(char * args);
static void ext2_command_stat(char * args);
static unsigned int ext2_partition_start_lba(void);
static unsigned int ext2_superblock_lba(void);
static int ext2_read_superblock(ext2_superblock * out_superblock);
static unsigned int ext2_block_size(const ext2_superblock * superblock);
static unsigned int ext2_block_to_lba(const ext2_superblock * superblock,
        unsigned int block_number);
static int ext2_read_block(const ext2_superblock * superblock,
        unsigned int block_number, void * buffer);
static int ext2_read_group_descriptor(const ext2_superblock * superblock,
        unsigned int group_index, ext2_group_descriptor * out_group_desc);
static int ext2_read_inode(const ext2_superblock * superblock,
        unsigned int inode_number, ext2_inode * out_inode);
static int ext2_lookup_name_in_directory(const ext2_superblock * superblock,
        const ext2_inode * directory_inode, const char * name,
        unsigned int * out_inode_number, unsigned char * out_file_type);
static int ext2_resolve_path_internal(const ext2_superblock * superblock,
        const char * path, unsigned int * out_inode_number,
        ext2_inode * out_inode, unsigned char * out_file_type);
static void ext2_list_directory(const ext2_superblock * superblock,
        const ext2_inode * directory_inode);
static int ext2_read_file_from_inode(const ext2_superblock * superblock,
        const ext2_inode * inode, void * buffer, unsigned int buffer_size,
        unsigned int * out_size);
static int ext2_list_directory_to_buffer(const ext2_superblock * superblock,
        const ext2_inode * directory_inode, char * buffer,
        unsigned int buffer_size, unsigned int * out_size);
static const char * ext2_file_type_name(unsigned char file_type);
static const char * ext2_inode_kind(const ext2_inode * inode);
static void ext2_print_inode_summary(const char * path, unsigned int inode_number,
        const ext2_inode * inode);
static char * ext2_skip_slashes(char * text);

void setup_ext2(void) {
    ext2_ready = 0;
    memset(&ext2_superblock_cache, 0, sizeof(ext2_superblock_cache));

    if (!disk_is_ready()) {
        serial_printf("[ext2] setup skipped: disk is not ready\n");
        return;
    }

    if (!ext2_read_superblock(&ext2_superblock_cache)) {
        serial_printf("[ext2] setup failed: superblock unavailable\n");
        return;
    }

    ext2_ready = 1;
    serial_printf("[ext2] setup complete block_size=%u inode_size=%u\n",
            ext2_block_size(&ext2_superblock_cache),
            ext2_superblock_cache.inode_size);
}

void register_ext2_monitor_commands(void) {
    monitor_register_command("ext2ls", ext2_command_ls,
            "list entries from an ext2 directory path, default /");
    monitor_register_command("ext2cat", ext2_command_cat,
            "read a text file from ext2, default /boot/grub/menu.lst");
    monitor_register_command("ext2stat", ext2_command_stat,
            "show inode metadata for an ext2 path");
}

int ext2_is_ready(void) {
    return ext2_ready;
}

int ext2_resolve_path(const char * path, unsigned int * out_inode_number,
        unsigned short * out_mode, unsigned int * out_size) {
    ext2_inode inode;
    unsigned int inode_number;
    unsigned char file_type;

    if (!ext2_ready) {
        return 0;
    }

    if (!ext2_resolve_path_internal(&ext2_superblock_cache, path,
            &inode_number, &inode, &file_type)) {
        return 0;
    }

    (void)file_type;

    if (out_inode_number != 0) {
        *out_inode_number = inode_number;
    }
    if (out_mode != 0) {
        *out_mode = inode.mode;
    }
    if (out_size != 0) {
        *out_size = inode.size_lo;
    }
    return 1;
}

int ext2_read_file(const char * path, void * buffer, unsigned int buffer_size,
        unsigned int * out_size) {
    ext2_inode inode;
    unsigned int inode_number;
    unsigned char file_type;

    if (!ext2_ready || path == 0 || buffer == 0 || buffer_size == 0) {
        return 0;
    }

    if (!ext2_resolve_path_internal(&ext2_superblock_cache, path,
            &inode_number, &inode, &file_type)) {
        serial_printf("[ext2] read_file path lookup failed path=%s\n", path);
        return 0;
    }

    if ((inode.mode & EXT2_MODE_FILE) != EXT2_MODE_FILE) {
        klog_printf("ext2: path is not a regular file: %s\n", path);
        serial_printf("[ext2] read_file rejected non-regular path=%s inode=%u mode=0x%x\n",
                path, inode_number, inode.mode);
        return 0;
    }

    return ext2_read_file_from_inode(&ext2_superblock_cache, &inode, buffer,
            buffer_size, out_size);
}

int ext2_list_dir(const char * path, void * buffer, unsigned int buffer_size,
        unsigned int * out_size) {
    ext2_inode inode;
    unsigned int inode_number;
    unsigned char file_type;

    if (!ext2_ready || path == 0 || buffer == 0 || buffer_size == 0) {
        return 0;
    }

    if (!ext2_resolve_path_internal(&ext2_superblock_cache, path,
            &inode_number, &inode, &file_type)) {
        serial_printf("[ext2] list_dir path lookup failed path=%s\n", path);
        return 0;
    }

    if ((inode.mode & EXT2_MODE_DIR) != EXT2_MODE_DIR) {
        serial_printf("[ext2] list_dir rejected non-directory path=%s inode=%u mode=0x%x\n",
                path, inode_number, inode.mode);
        return 0;
    }

    return ext2_list_directory_to_buffer(&ext2_superblock_cache, &inode,
            (char *)buffer, buffer_size, out_size);
}

static void ext2_command_ls(char * args) {
    const char * path;
    ext2_inode inode;
    unsigned int inode_number;
    unsigned char file_type;

    path = EXT2_DEFAULT_LS_PATH;
    if (args != 0 && args[0] != 0) {
        path = args;
    }

    if (!ext2_ready) {
        klog_printf("ext2: reader is not ready\n");
        return;
    }

    if (!ext2_resolve_path_internal(&ext2_superblock_cache, path,
            &inode_number, &inode, &file_type)) {
        klog_printf("ext2: path not found: %s\n", path);
        return;
    }

    if ((inode.mode & EXT2_MODE_DIR) != EXT2_MODE_DIR) {
        klog_printf("ext2: path is not a directory: %s\n", path);
        return;
    }

    klog_printf("ext2: listing %s inode=%u\n", path, inode_number);
    ext2_list_directory(&ext2_superblock_cache, &inode);
}

static void ext2_command_cat(char * args) {
    const char * path;
    unsigned int size;

    path = EXT2_DEFAULT_CAT_PATH;
    if (args != 0 && args[0] != 0) {
        path = args;
    }

    if (!ext2_ready) {
        klog_printf("ext2: reader is not ready\n");
        return;
    }

    memset(ext2_read_buffer, 0, sizeof(ext2_read_buffer));
    if (!ext2_read_file(path, ext2_read_buffer, EXT2_FILE_READ_MAX, &size)) {
        klog_printf("ext2: could not read file: %s\n", path);
        return;
    }

    ext2_read_buffer[size] = 0;
    klog_printf("ext2: read path=%s size=%u\n", path, size);
    klog_printf("%s\n", ext2_read_buffer);
}

static void ext2_command_stat(char * args) {
    const char * path;
    ext2_inode inode;
    unsigned int inode_number;
    unsigned char file_type;

    path = EXT2_DEFAULT_CAT_PATH;
    if (args != 0 && args[0] != 0) {
        path = args;
    }

    if (!ext2_ready) {
        klog_printf("ext2: reader is not ready\n");
        return;
    }

    if (!ext2_resolve_path_internal(&ext2_superblock_cache, path,
            &inode_number, &inode, &file_type)) {
        klog_printf("ext2: path not found: %s\n", path);
        return;
    }

    (void)file_type;
    ext2_print_inode_summary(path, inode_number, &inode);
}

static unsigned int ext2_partition_start_lba(void) {
    return 63;
}

static unsigned int ext2_superblock_lba(void) {
    return ext2_partition_start_lba()
            + (EXT2_SUPERBLOCK_OFFSET / DISK_SECTOR_SIZE);
}

static int ext2_read_superblock(ext2_superblock * out_superblock) {
    unsigned char superblock_buffer[EXT2_SUPERBLOCK_SIZE];
    unsigned int superblock_lba;

    if (out_superblock == 0) {
        return 0;
    }

    superblock_lba = ext2_superblock_lba();
    memset(superblock_buffer, 0, sizeof(superblock_buffer));
    if (!disk_read_sectors(superblock_lba, 2, superblock_buffer)) {
        return 0;
    }

    memcpy(out_superblock, superblock_buffer, sizeof(ext2_superblock));
    if (out_superblock->magic != EXT2_SUPERBLOCK_MAGIC) {
        return 0;
    }

    return 1;
}

static unsigned int ext2_block_size(const ext2_superblock * superblock) {
    return 1024U << superblock->log_block_size;
}

static unsigned int ext2_block_to_lba(const ext2_superblock * superblock,
        unsigned int block_number) {
    unsigned int block_size;
    unsigned int fs_offset;

    block_size = ext2_block_size(superblock);
    fs_offset = block_number * block_size;
    return ext2_partition_start_lba() + (fs_offset / DISK_SECTOR_SIZE);
}

static int ext2_read_block(const ext2_superblock * superblock,
        unsigned int block_number, void * buffer) {
    unsigned int block_size;
    unsigned int sector_count;
    unsigned int lba;

    block_size = ext2_block_size(superblock);
    if (block_size > sizeof(ext2_block_buffer)) {
        return 0;
    }

    sector_count = block_size / DISK_SECTOR_SIZE;
    lba = ext2_block_to_lba(superblock, block_number);
    return disk_read_sectors(lba, (unsigned char) sector_count, buffer);
}

static int ext2_read_group_descriptor(const ext2_superblock * superblock,
        unsigned int group_index, ext2_group_descriptor * out_group_desc) {
    unsigned int block_size;
    unsigned int descriptor_block;
    unsigned int descriptor_offset;

    if (out_group_desc == 0) {
        return 0;
    }

    block_size = ext2_block_size(superblock);
    descriptor_block = superblock->first_data_block + 1
            + ((group_index * sizeof(ext2_group_descriptor)) / block_size);
    descriptor_offset = (group_index * sizeof(ext2_group_descriptor))
            % block_size;

    memset(ext2_block_buffer, 0, sizeof(ext2_block_buffer));
    if (!ext2_read_block(superblock, descriptor_block, ext2_block_buffer)) {
        return 0;
    }

    memcpy(out_group_desc, ext2_block_buffer + descriptor_offset,
            sizeof(ext2_group_descriptor));
    return 1;
}

static int ext2_read_inode(const ext2_superblock * superblock,
        unsigned int inode_number, ext2_inode * out_inode) {
    ext2_group_descriptor group_desc;
    unsigned int inode_size;
    unsigned int zero_based_inode;
    unsigned int group_index;
    unsigned int index_in_group;
    unsigned int block_size;
    unsigned int inode_offset;
    unsigned int block_index;
    unsigned int offset_in_block;

    if (out_inode == 0 || inode_number == 0) {
        return 0;
    }

    inode_size = superblock->inode_size;
    if (inode_size == 0) {
        inode_size = 128;
    }

    zero_based_inode = inode_number - 1;
    group_index = zero_based_inode / superblock->inodes_per_group;
    index_in_group = zero_based_inode % superblock->inodes_per_group;

    if (!ext2_read_group_descriptor(superblock, group_index, &group_desc)) {
        return 0;
    }

    block_size = ext2_block_size(superblock);
    inode_offset = index_in_group * inode_size;
    block_index = inode_offset / block_size;
    offset_in_block = inode_offset % block_size;

    memset(ext2_block_buffer, 0, sizeof(ext2_block_buffer));
    if (!ext2_read_block(superblock, group_desc.inode_table + block_index,
            ext2_block_buffer)) {
        return 0;
    }

    memcpy(out_inode, ext2_block_buffer + offset_in_block, sizeof(ext2_inode));
    return 1;
}

static int ext2_lookup_name_in_directory(const ext2_superblock * superblock,
        const ext2_inode * directory_inode, const char * name,
        unsigned int * out_inode_number, unsigned char * out_file_type) {
    unsigned int block_size;
    unsigned int block_index;
    unsigned int remaining_size;

    if (directory_inode == 0 || name == 0 || name[0] == 0) {
        return 0;
    }

    if ((directory_inode->mode & EXT2_MODE_DIR) != EXT2_MODE_DIR) {
        return 0;
    }

    block_size = ext2_block_size(superblock);
    remaining_size = directory_inode->size_lo;

    for (block_index = 0; block_index < 12 && remaining_size > 0; block_index++) {
        unsigned int offset;
        unsigned int block_limit;

        if (directory_inode->block[block_index] == 0) {
            continue;
        }

        memset(ext2_block_buffer, 0, sizeof(ext2_block_buffer));
        if (!ext2_read_block(superblock, directory_inode->block[block_index],
                ext2_block_buffer)) {
            return 0;
        }

        block_limit = remaining_size;
        if (block_limit > block_size) {
            block_limit = block_size;
        }

        offset = 0;
        while (offset + EXT2_DIR_ENTRY_HEADER_SIZE <= block_limit) {
            ext2_dir_entry * entry;
            unsigned int copy_len;

            entry = (ext2_dir_entry *) (ext2_block_buffer + offset);
            if (entry->rec_len == 0) {
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

                if (strcmp(ext2_name_buffer, name) == 0) {
                    if (out_inode_number != 0) {
                        *out_inode_number = entry->inode;
                    }
                    if (out_file_type != 0) {
                        *out_file_type = entry->file_type;
                    }
                    return 1;
                }
            }

            offset += entry->rec_len;
        }

        if (remaining_size > block_size) {
            remaining_size -= block_size;
        } else {
            remaining_size = 0;
        }
    }

    return 0;
}

static int ext2_resolve_path_internal(const ext2_superblock * superblock,
        const char * path, unsigned int * out_inode_number,
        ext2_inode * out_inode, unsigned char * out_file_type) {
    char * cursor;
    unsigned int current_inode_number;
    ext2_inode current_inode;
    unsigned char current_file_type;

    if (path == 0 || path[0] != '/') {
        return 0;
    }

    memset(ext2_path_buffer, 0, sizeof(ext2_path_buffer));
    strcpy(ext2_path_buffer, path);
    cursor = ext2_path_buffer;

    current_inode_number = EXT2_ROOT_INODE;
    if (!ext2_read_inode(superblock, current_inode_number, &current_inode)) {
        return 0;
    }
    current_file_type = 2;

    cursor = ext2_skip_slashes(cursor);
    if (*cursor == 0) {
        if (out_inode_number != 0) {
            *out_inode_number = current_inode_number;
        }
        if (out_inode != 0) {
            memcpy(out_inode, &current_inode, sizeof(ext2_inode));
        }
        if (out_file_type != 0) {
            *out_file_type = current_file_type;
        }
        return 1;
    }

    while (*cursor != 0) {
        char * slash;
        unsigned int next_inode_number;
        unsigned char next_file_type;

        slash = strchr(cursor, '/');
        if (slash != 0) {
            *slash = 0;
        }

        memset(ext2_component_buffer, 0, sizeof(ext2_component_buffer));
        strcpy(ext2_component_buffer, cursor);

        if (!ext2_lookup_name_in_directory(superblock, &current_inode,
                ext2_component_buffer, &next_inode_number, &next_file_type)) {
            return 0;
        }

        if (!ext2_read_inode(superblock, next_inode_number, &current_inode)) {
            return 0;
        }

        current_inode_number = next_inode_number;
        current_file_type = next_file_type;

        if (slash == 0) {
            break;
        }

        cursor = ext2_skip_slashes(slash + 1);
    }

    if (out_inode_number != 0) {
        *out_inode_number = current_inode_number;
    }
    if (out_inode != 0) {
        memcpy(out_inode, &current_inode, sizeof(ext2_inode));
    }
    if (out_file_type != 0) {
        *out_file_type = current_file_type;
    }
    return 1;
}

static void ext2_list_directory(const ext2_superblock * superblock,
        const ext2_inode * directory_inode) {
    unsigned int block_size;
    unsigned int block_index;
    unsigned int remaining_size;

    block_size = ext2_block_size(superblock);
    remaining_size = directory_inode->size_lo;

    for (block_index = 0; block_index < 12 && remaining_size > 0; block_index++) {
        unsigned int offset;
        unsigned int block_limit;

        if (directory_inode->block[block_index] == 0) {
            continue;
        }

        memset(ext2_block_buffer, 0, sizeof(ext2_block_buffer));
        if (!ext2_read_block(superblock, directory_inode->block[block_index],
                ext2_block_buffer)) {
            klog_printf("ext2: failed to read directory block %u\n",
                    directory_inode->block[block_index]);
            return;
        }

        block_limit = remaining_size;
        if (block_limit > block_size) {
            block_limit = block_size;
        }

        offset = 0;
        while (offset + EXT2_DIR_ENTRY_HEADER_SIZE <= block_limit) {
            ext2_dir_entry * entry;
            unsigned int copy_len;

            entry = (ext2_dir_entry *) (ext2_block_buffer + offset);
            if (entry->rec_len == 0) {
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

                klog_printf("ext2: inode=%u type=%s name=%s\n",
                        entry->inode, ext2_file_type_name(entry->file_type),
                        ext2_name_buffer);
            }

            offset += entry->rec_len;
        }

        if (remaining_size > block_size) {
            remaining_size -= block_size;
        } else {
            remaining_size = 0;
        }
    }
}

static int ext2_list_directory_to_buffer(const ext2_superblock * superblock,
        const ext2_inode * directory_inode, char * buffer,
        unsigned int buffer_size, unsigned int * out_size) {
    unsigned int block_size;
    unsigned int block_index;
    unsigned int remaining_size;
    unsigned int copied;

    if (buffer == 0 || buffer_size == 0) {
        return 0;
    }

    block_size = ext2_block_size(superblock);
    remaining_size = directory_inode->size_lo;
    copied = 0;
    memset(buffer, 0, buffer_size);

    for (block_index = 0; block_index < 12 && remaining_size > 0; block_index++) {
        unsigned int offset;
        unsigned int block_limit;

        if (directory_inode->block[block_index] == 0) {
            continue;
        }

        memset(ext2_block_buffer, 0, sizeof(ext2_block_buffer));
        if (!ext2_read_block(superblock, directory_inode->block[block_index],
                ext2_block_buffer)) {
            return 0;
        }

        block_limit = remaining_size;
        if (block_limit > block_size) {
            block_limit = block_size;
        }

        offset = 0;
        while (offset + EXT2_DIR_ENTRY_HEADER_SIZE <= block_limit) {
            ext2_dir_entry * entry;
            unsigned int copy_len;

            entry = (ext2_dir_entry *) (ext2_block_buffer + offset);
            if (entry->rec_len == 0) {
                break;
            }

            if (entry->inode != 0) {
                copy_len = entry->name_len;
                if (copy_len > EXT2_NAME_MAX) {
                    copy_len = EXT2_NAME_MAX;
                }

                if ((copied + copy_len + 1) >= buffer_size) {
                    if (out_size != 0) {
                        *out_size = copied;
                    }
                    return 1;
                }

                memcpy(buffer + copied, entry->name, copy_len);
                copied += copy_len;
                buffer[copied++] = '\n';
                buffer[copied] = 0;
            }

            offset += entry->rec_len;
        }

        if (remaining_size > block_size) {
            remaining_size -= block_size;
        } else {
            remaining_size = 0;
        }
    }

    if (out_size != 0) {
        *out_size = copied;
    }
    return 1;
}

static int ext2_read_file_from_inode(const ext2_superblock * superblock,
        const ext2_inode * inode, void * buffer, unsigned int buffer_size,
        unsigned int * out_size) {
    unsigned int block_size;
    unsigned int block_index;
    unsigned int remaining_size;
    unsigned int copied;
    unsigned char * out;

    if (inode == 0 || buffer == 0) {
        return 0;
    }

    block_size = ext2_block_size(superblock);
    if (buffer_size > EXT2_FILE_READ_MAX) {
        buffer_size = EXT2_FILE_READ_MAX;
    }

    remaining_size = inode->size_lo;
    if (remaining_size > buffer_size) {
        remaining_size = buffer_size;
    }

    copied = 0;
    out = (unsigned char *) buffer;

    for (block_index = 0; block_index < 12 && remaining_size > 0; block_index++) {
        unsigned int chunk_size;

        if (inode->block[block_index] == 0) {
            break;
        }

        memset(ext2_block_buffer, 0, sizeof(ext2_block_buffer));
        if (!ext2_read_block(superblock, inode->block[block_index],
                ext2_block_buffer)) {
            return 0;
        }

        chunk_size = remaining_size;
        if (chunk_size > block_size) {
            chunk_size = block_size;
        }

        memcpy(out + copied, ext2_block_buffer, chunk_size);
        copied += chunk_size;
        remaining_size -= chunk_size;
    }

    if (out_size != 0) {
        *out_size = copied;
    }
    return 1;
}

static const char * ext2_file_type_name(unsigned char file_type) {
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

static const char * ext2_inode_kind(const ext2_inode * inode) {
    if ((inode->mode & EXT2_MODE_DIR) == EXT2_MODE_DIR) {
        return "dir";
    }
    if ((inode->mode & EXT2_MODE_FILE) == EXT2_MODE_FILE) {
        return "file";
    }
    return "other";
}

static void ext2_print_inode_summary(const char * path, unsigned int inode_number,
        const ext2_inode * inode) {
    klog_printf("ext2: path=%s inode=%u kind=%s mode=0x%x size=%u links=%u blocks=%u\n",
            path, inode_number, ext2_inode_kind(inode), inode->mode,
            inode->size_lo, inode->links_count, inode->blocks);
    klog_printf("ext2: direct_blocks=%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n",
            inode->block[0], inode->block[1], inode->block[2], inode->block[3],
            inode->block[4], inode->block[5], inode->block[6], inode->block[7],
            inode->block[8], inode->block[9], inode->block[10], inode->block[11]);
}

static char * ext2_skip_slashes(char * text) {
    while (text != 0 && *text == '/') {
        text++;
    }
    return text;
}
