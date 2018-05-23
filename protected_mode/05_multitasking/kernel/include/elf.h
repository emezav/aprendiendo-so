/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Contiene las definiciones de las estructuras de datos y las
 * rutinas requeridas para interpretar ejecutables en el formato ELF.
 */

#ifndef ELF_H_
#define ELF_H_

/**
 * @brief Encabezado de un archvo que cumple con la especificación ELF
 */
typedef struct {
	unsigned char  ident[16];
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

/**
 * @brief Estructura de datos para un encabezado de programa dentro del archivo
 * ELF
 */
typedef struct {
	unsigned int type;
	unsigned int offset;
	unsigned int virtual_address;
	unsigned int physical_address;
	unsigned int file_size;
	unsigned int memory_size;
	unsigned int flags;
	unsigned int align;
} program_header;

/** @brief tipo de program header que debe ser cargado en memoria.
 * Ver la especificacion ELF, en la seccion Program Header el apartado
 * Segment Types
 */
#define ELF_PT_LOAD 0x1

/** @brief Constante para un encabezado de programa ejecutable */
#define ELF_PH_EXECUTE 0x01
/** @brief Constante para un encabezado de programa de escritura */
#define ELF_PH_WRITE 0x02
/** @brief Constante para un encabezado de programa de lectura*/
#define ELF_PH_READ 0x03

#endif /* ELF_H_ */
