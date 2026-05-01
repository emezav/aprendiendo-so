/**
 * @file
 * @ingroup kernel_code
 * @brief Definiciones minimas para interpretar ejecutables ELF32.
 */

#ifndef ELF_H_
#define ELF_H_

#define ELF_MAGIC_0 0x7F
#define ELF_MAGIC_1 'E'
#define ELF_MAGIC_2 'L'
#define ELF_MAGIC_3 'F'

#define ELF_CLASS_32 1
#define ELF_DATA_LSB 1

#define ELF_TYPE_EXEC 2
#define ELF_MACHINE_386 3

#define ELF_PT_LOAD 1

typedef struct __attribute__((packed)) elf_header {
    unsigned char ident[16];
    unsigned short type;
    unsigned short machine;
    unsigned int version;
    unsigned int entry;
    unsigned int program_header_offset;
    unsigned int section_header_offset;
    unsigned int flags;
    unsigned short header_size;
    unsigned short program_header_size;
    unsigned short program_header_count;
    unsigned short section_header_size;
    unsigned short section_header_count;
    unsigned short section_header_string_index;
} elf_header;

typedef struct __attribute__((packed)) program_header {
    unsigned int type;
    unsigned int offset;
    unsigned int virtual_address;
    unsigned int physical_address;
    unsigned int file_size;
    unsigned int memory_size;
    unsigned int flags;
    unsigned int align;
} program_header;

#endif /* ELF_H_ */
