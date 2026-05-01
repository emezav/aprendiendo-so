/**
 * @file
 * @ingroup kernel_code
 * @brief Cargador minimo de ELF32 desde ext2 y salto al punto de entrada.
 */

#include <asm.h>
#include <elf.h>
#include <ext2.h>
#include <klog.h>
#include <monitor.h>
#include <paging.h>
#include <physmem.h>
#include <pm.h>
#include <serial.h>
#include <string.h>
#include <tss.h>
#include <userexec.h>

#define USEREXEC_DEFAULT_PATH "/boot/hello.elf"
#define USEREXEC_ELF_MAX_SIZE EXT2_FILE_READ_MAX
#define USEREXEC_STACK_BASE (USER_VIRT_START + 0x00020000)
#define USEREXEC_STACK_TOP (USEREXEC_STACK_BASE + PAGE_SIZE)

static unsigned char userexec_elf_buffer[USEREXEC_ELF_MAX_SIZE];
static unsigned int userexec_entry_point;

static void userexec_command_info(char * args);
static void userexec_command_run(char * args);
static int userexec_load_file(const char * path, unsigned int * out_size);
static int userexec_validate_elf(const unsigned char * file_data,
        unsigned int file_size, const elf_header ** out_header);
static void userexec_dump_elf_info(const elf_header * header,
        const program_header * program_headers);
static int userexec_map_segment(const unsigned char * file_data,
        unsigned int file_size, const program_header * program_header);
static int userexec_prepare_stack(void);
static void userexec_reset_region(unsigned int start_vaddr,
        unsigned int end_vaddr);
static void userexec_enter(unsigned int entry_point);

void setup_user_exec(void) {
    userexec_entry_point = 0;
}

void register_user_exec_monitor_commands(void) {
    monitor_register_command("elfinfo", userexec_command_info,
            "read an ELF from ext2 and show its headers, default /boot/hello.elf");
    monitor_register_command("runelf", userexec_command_run,
            "load an ELF from ext2 and execute it in mode de usuario");
}

static void userexec_command_info(char * args) {
    const char * path;
    const elf_header * header;
    const program_header * program_headers;
    unsigned int file_size;

    path = USEREXEC_DEFAULT_PATH;
    if (args != 0 && args[0] != 0) {
        path = args;
    }

    serial_printf("[userexec] elfinfo start path=%s\n", path);

    if (!userexec_load_file(path, &file_size)) {
        klog_printf("userexec: could not read ELF: %s\n", path);
        return;
    }

    serial_printf("[userexec] elfinfo loaded path=%s size=%u\n", path, file_size);

    if (!userexec_validate_elf(userexec_elf_buffer, file_size, &header)) {
        klog_printf("userexec: invalid ELF: %s\n", path);
        serial_printf("[userexec] elfinfo invalid path=%s\n", path);
        return;
    }

    program_headers = (const program_header *)(userexec_elf_buffer
            + header->program_header_offset);
    klog_printf("userexec: elfinfo path=%s size=%u\n", path, file_size);
    userexec_dump_elf_info(header, program_headers);
}

static void userexec_command_run(char * args) {
    const char * path;
    const elf_header * header;
    const program_header * program_headers;
    unsigned int file_size;
    unsigned int i;

    path = USEREXEC_DEFAULT_PATH;
    if (args != 0 && args[0] != 0) {
        path = args;
    }

    serial_printf("[userexec] runelf start path=%s\n", path);

    if (user_code_selector == 0 || user_data_selector == 0) {
        klog_printf("userexec: user segments are not initialized\n");
        return;
    }

    if (kernel_tss_selector == 0) {
        klog_printf("userexec: tss is not initialized\n");
        return;
    }

    if (!userexec_load_file(path, &file_size)) {
        klog_printf("userexec: could not read ELF: %s\n", path);
        return;
    }

    serial_printf("[userexec] runelf loaded path=%s size=%u\n", path, file_size);

    if (!userexec_validate_elf(userexec_elf_buffer, file_size, &header)) {
        klog_printf("userexec: invalid ELF: %s\n", path);
        serial_printf("[userexec] runelf invalid path=%s\n", path);
        return;
    }

    program_headers = (const program_header *)(userexec_elf_buffer
            + header->program_header_offset);

    for (i = 0; i < header->program_header_count; i++) {
        if (!userexec_map_segment(userexec_elf_buffer, file_size,
                &program_headers[i])) {
            klog_printf("userexec: could not map program header %u\n", i);
            return;
        }
    }

    if (!userexec_prepare_stack()) {
        klog_printf("userexec: could not prepare user stack\n");
        return;
    }

    userexec_entry_point = header->entry;
    klog_printf("userexec: running %s entry=0x%x\n", path, userexec_entry_point);
    serial_printf("[userexec] run path=%s entry=0x%x phnum=%u\n", path,
            userexec_entry_point, header->program_header_count);
    userexec_enter(userexec_entry_point);

    klog_printf("userexec: unexpected return to monitor command path\n");
}

static int userexec_load_file(const char * path, unsigned int * out_size) {
    unsigned int file_size;

    memset(userexec_elf_buffer, 0, sizeof(userexec_elf_buffer));
    if (!ext2_read_file(path, userexec_elf_buffer, sizeof(userexec_elf_buffer),
            &file_size)) {
        serial_printf("[userexec] ext2_read_file failed path=%s\n", path);
        return 0;
    }

    if (out_size != 0) {
        *out_size = file_size;
    }

    serial_printf("[userexec] read path=%s size=%u\n", path, file_size);
    return 1;
}

static int userexec_validate_elf(const unsigned char * file_data,
        unsigned int file_size, const elf_header ** out_header) {
    const elf_header * header;

    if (file_data == 0 || file_size < sizeof(elf_header)) {
        return 0;
    }

    header = (const elf_header *) file_data;
    if (header->ident[0] != ELF_MAGIC_0 || header->ident[1] != ELF_MAGIC_1
            || header->ident[2] != ELF_MAGIC_2
            || header->ident[3] != ELF_MAGIC_3) {
        return 0;
    }

    if (header->ident[4] != ELF_CLASS_32 || header->ident[5] != ELF_DATA_LSB) {
        return 0;
    }

    if (header->type != ELF_TYPE_EXEC || header->machine != ELF_MACHINE_386) {
        return 0;
    }

    if (header->program_header_size != sizeof(program_header)) {
        return 0;
    }

    if (header->program_header_offset
            + (header->program_header_count * sizeof(program_header))
            > file_size) {
        return 0;
    }

    if (out_header != 0) {
        *out_header = header;
    }
    return 1;
}

static void userexec_dump_elf_info(const elf_header * header,
        const program_header * program_headers) {
    unsigned int i;

    klog_printf("userexec: elf entry=0x%x phoff=%u phnum=%u phentsize=%u\n",
            header->entry, header->program_header_offset,
            header->program_header_count, header->program_header_size);

    for (i = 0; i < header->program_header_count; i++) {
        klog_printf("userexec: ph[%u] type=%u off=0x%x vaddr=0x%x filesz=%u memsz=%u flags=0x%x align=0x%x\n",
                i, program_headers[i].type, program_headers[i].offset,
                program_headers[i].virtual_address,
                program_headers[i].file_size, program_headers[i].memory_size,
                program_headers[i].flags, program_headers[i].align);
    }
}

static int userexec_map_segment(const unsigned char * file_data,
        unsigned int file_size, const program_header * program_header) {
    unsigned int start_vaddr;
    unsigned int end_vaddr;
    unsigned int current_vaddr;
    unsigned int offset;

    if (program_header->type != ELF_PT_LOAD) {
        return 1;
    }

    if (program_header->offset + program_header->file_size > file_size) {
        return 0;
    }

    start_vaddr = ROUND_DOWN_TO_PAGE(program_header->virtual_address);
    end_vaddr = ROUND_UP_TO_PAGE(program_header->virtual_address
            + program_header->memory_size);

    serial_printf("[userexec] map segment type=%u start=0x%x end=0x%x off=0x%x filesz=%u memsz=%u\n",
            program_header->type, start_vaddr, end_vaddr,
            program_header->offset, program_header->file_size,
            program_header->memory_size);

    userexec_reset_region(start_vaddr, end_vaddr);

    for (current_vaddr = start_vaddr; current_vaddr < end_vaddr;
            current_vaddr += PAGE_SIZE) {
        unsigned int frame;

        frame = allocate_frame();
        if (!frame) {
            serial_printf("[userexec] allocate_frame failed vaddr=0x%x\n",
                    current_vaddr);
            return 0;
        }

        if (!map_user_page(current_vaddr, frame)) {
            serial_printf("[userexec] map_user_page failed vaddr=0x%x frame=0x%x\n",
                    current_vaddr, frame);
            free_frame(frame);
            return 0;
        }

        memset((void *) current_vaddr, 0, PAGE_SIZE);
    }

    for (offset = 0; offset < program_header->file_size; offset++) {
        unsigned int dst_vaddr;

        dst_vaddr = program_header->virtual_address + offset;
        *((unsigned char *) dst_vaddr) =
                file_data[program_header->offset + offset];
    }

    serial_printf("[userexec] mapped load segment vaddr=0x%x filesz=%u memsz=%u\n",
            program_header->virtual_address,
            program_header->file_size, program_header->memory_size);
    return 1;
}

static int userexec_prepare_stack(void) {
    unsigned int frame;

    userexec_reset_region(USEREXEC_STACK_BASE, USEREXEC_STACK_TOP);

    frame = allocate_frame();
    if (!frame) {
        serial_printf("[userexec] allocate_frame failed for stack\n");
        return 0;
    }

    if (!map_user_page(USEREXEC_STACK_BASE, frame)) {
        serial_printf("[userexec] map_user_page failed for stack frame=0x%x\n",
                frame);
        free_frame(frame);
        return 0;
    }

    memset((void *) USEREXEC_STACK_BASE, 0, PAGE_SIZE);
    return 1;
}

static void userexec_reset_region(unsigned int start_vaddr,
        unsigned int end_vaddr) {
    unsigned int current_vaddr;

    for (current_vaddr = ROUND_DOWN_TO_PAGE(start_vaddr);
            current_vaddr < ROUND_UP_TO_PAGE(end_vaddr);
            current_vaddr += PAGE_SIZE) {
        destroy_page(current_vaddr);
    }
}

static void userexec_enter(unsigned int entry_point) {
    unsigned int user_code;
    unsigned int user_data;

    user_code = user_code_selector | 0x03;
    user_data = user_data_selector | 0x03;

    inline_assembly(".intel_syntax noprefix\n\t"
                    "mov ds, %w0\n\t"
                    "mov es, %w0\n\t"
                    "mov fs, %w0\n\t"
                    "mov gs, %w0\n\t"
                    "cli\n\t"
                    "push %0\n\t"
                    "push %1\n\t"
                    "push %2\n\t"
                    "push %3\n\t"
                    "push %4\n\t"
                    "iret\n\t"
                    ".att_syntax prefix\n\t"
                    :
                    : "r"(user_data),
                      "r"(USEREXEC_STACK_TOP),
                      "r"(IF_ENABLE),
                      "r"(user_code),
                      "r"(entry_point)
                    : "memory");
}
