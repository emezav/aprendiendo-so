/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Contiene algunas constantes necesarias para  el kernel
 * relacionadas con la especificación Multiboot
 * @see http://www.gnu.org/software/grub/manual/multiboot/multiboot.html
 */

#ifndef MULTIBOOT_H_
#define MULTIBOOT_H_

/* Estas FLAGS se pasan a GRUB. Ver especificacion Multiboot. */
/** @brief Alinear los módulos cargados a límites de página */
#define MULTIBOOT_PAGE_ALIGN 1<<0
/** @brief Proporcionar al kernel información de la memoria disponible */
#define MULTIBOOT_MEMORY_INFO 1<<1
/** @brief Número mágico de la especificacón multiboot */
#define MULTIBOOT_HEADER_MAGIC 0x1BADB002
/** @brief Constante que incluye las flags que se pasarán a GRUB */
#define MULTIBOOT_HEADER_FLAGS MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
/** @brief Constante de suma de chequeo*/
#define MULTIBOOT_CHECKSUM -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
/** @brief Número mágico que el cargador de arranque almacena en el registro
 * EAX para indicar que es compatible con la especificación Multiboot */
#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002
/** @brief Permite determinar si se han cargado modulos junto con el kernel */
#define MODULES_LOADED 0X08
/** @brief Permite determinar si el cargador proporciona el mapa de memoria */
#define MEMORY_MAP_PRESENT 0X40

/* No incluir de aqui en adelante si se incluye este archivo desde codigo
 * en ensamblador */
#ifndef ASM

/** @brief Definición del tipo de datos del Encabezado Multiboot */
typedef struct multiboot_header_struct {
	/** @brief Número mágico */
	unsigned int magic;
	/** @brief Flags pasadas a GRUB para solicitar información / servicios. */
	unsigned int flags;
	/** @brief Número de chequeo. Este valor debe ser igual a la suma de
	 * magic y flags. */
	unsigned int checksum;
} multiboot_header_t;

/** @brief Tabla de símbolos usadas en el formato a.out*/
typedef struct aout_symbol_table
{
	unsigned int tabsize;
	unsigned int strsize;
	unsigned int addr;
	unsigned int reserved;
} aout_symbol_table_t;

/** @brief Tabla de encabezados de sección del kernel en el formato elf */
typedef struct elf_section_header_table
{
	unsigned int num;
	unsigned int size;
	unsigned int addr;
	unsigned int shndx;
}elf_section_header_table_t;

/** @brief Estructura de datos que almacena la información de una
 * región de memoria dentro del mapa de memoria  proporcionado por GRUB. */
typedef struct memory_map
{
 /** @brief Campo no usado */
 unsigned int entry_size;
 /** @brief 32 bits menos significativos de la base de la región de memoria */
 unsigned int base_addr_low;
 /** @brief 32 bits más significativos de la base de la región de memoria */
 unsigned int base_addr_high;
 /** @brief 32 bits menos significativos del tamaño de la región de memoria */
 unsigned int length_low;
 /** @brief 32 bits más significativos del tamaño de la región de memoria */
 unsigned int length_high;
 /** @brief Tipo de área de memoria 1 = disponible, 2 = reservada */
 unsigned int type;
} memory_map_t;

/** @brief Estructura de datos que almacena la información de un módulo
 * cargado por GRUB. */
typedef struct mod_info  {
	/** @brief Dirección en la cual se cargó el módulo */
	unsigned int mod_start;
	/** @brief Tamaño en bytes del módulo cargado */
	unsigned int mod_end;
	/** @brief 32 Comando usado para cargar el módulo  */
	char * string;
	unsigned int always0;
}mod_info_t;

/** @brief Estructura de información Multiboot. Al cargar el kernel,
 * GRUB almacena en el registro EBX un apuntador a la dirección de memoria
 * en la que se encuentra esta estructura. */
typedef struct {
	/** @brief Versión e información de Multiboot. El kernel deberá comprobar
	 * sus bits para verificar si GRUB le pasó la información solicitada. */
	unsigned int flags;
	/** @brief Presente si flags[0] = 1 Memoria baja reportada por la BIOS*/
	unsigned int mem_lower;
	/** @brief Presente si flags[0] = 1 Memoria alta reportada por la BIOS*/
	unsigned int mem_upper;
	/** @brief Presente si flags[1] = 1 Dispositivo desde el cual se cargó
	 * el kernel. */
	unsigned int boot_device;
	/** @brief Presente si flags[2] = 1 Línea de comandos usada para cargar
	 * el kernel */
	unsigned int cmdline;
	/** @brief Presente si flags[3] = 1 Número de módulos cargados junto
	 * con el kernel */
	unsigned int mods_count;
	/** @brief Presente si flags[3] = 1 Dirección de memoria en la cual se
	 * encuentra la información de los módulos cargados por el kernel. */
	unsigned int mods_addr;

	/** @brief Presente si flags[4] = 1 o flags[5] = 1. Información de símbolos
	 * a.out o de secciones ELF del kernel cargado. */
	union {
		aout_symbol_table_t aout_symbol_table;
		elf_section_header_table_t elf_section_table;
	}syms;

	/** @brief Presente si flags[6] = 1. Tamaño del mapa de memoria creado
	 * por GRUB*/
	unsigned int mmap_length;
	/** @brief Presente si flags[6] = 1. Dirección física de la ubicación del
	 * mapa de memoria creado por GRUB. */
	unsigned int mmap_addr;

	/** @brief Presente si flags[7] = 1. Especifica el tamaño total de la
	 * estructura que describe los drives reportados por la BIOS.
	 */
	unsigned int drives_length;/* Presente si flags[7] = 1 */
	/** @brief Presente si flags[7] = 1. Especifica la dirección de memoria en
	 * la que se encuentra la estructura que describe los drivers reportados
	 * por la BIOS.
	 */
	unsigned int drives_addr;	/* Presente si flags[7] = 1 */
	/** @brief Presente si flags[8] = 1. Especifica la dirección de la tabla
	 * de configuración de la BIOS. */
	unsigned int config_table;
	/** @brief Presente si flags[9] = 1. Contiene la dirección de memoria
	 * en la cual se encuentra una cadena de caracteres con el nombre del
	 * cargador de arranque. */
	unsigned int boot_loader_name;

	/** @brief Presente si flags[10] = 1. Especifica la localización en la
	 * memoria de la tabla APM. */
	unsigned int apm_table;

	/** @brief Presente si flags[11] = 1. Contiene la información de control
	 * retornada por la función vbe 00
	 */
	unsigned int vbe_control_info;
	/** @brief Presente si flags[11] = 1. Contiene la información de modo
	 * retornada por la función vbe 00
	 */
	unsigned int vbe_mode_info;

	/** @brief Presente si flags[11] = 1. */
	unsigned int vbe_mode;

	/** @brief Presente si flags[11] = 1. */
	unsigned short vbe_interface_seg;
	/** @brief Presente si flags[11] = 1. */
	unsigned short vbe_interface_off;
	/** @brief Presente si flags[11] = 1. */
	unsigned short vbe_interface_len;

}multiboot_info_t;

/* @brief Variable que almacena la ubicación de la estructura multiboot en
 * memoria. Definida en start.S */
extern unsigned int multiboot_info_location;

#endif

#endif
