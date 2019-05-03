/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 * @brief Contiene las definiciones relacionadas con la paginación.
 */

#ifndef PAGING_H_
#define PAGING_H_

#include <pm.h>

/** @brief Bit para activar la paginación en el registro CR0 */
#define ENABLE_PAGING 0x80000000

/** @brief Tamaño de la unidad de asignación de memoria  (página, marco)*/
#define PAGE_SIZE 4096

/* @brief Número de entradas en el directorio de tablas de página */
#define PD_ENTRIES 1024

/* @brief Número de entradas en una tabla de página */
#define PT_ENTRIES 1024

/* @brief Bit 'P' en las entradas del directorio y tablas de página */
#define PG_PRESENT 1

/* @brief Bits para una entrada no usada en el directorio y la tabla de páginas
 */
#define PG_UNUSED 2

/* @brief Bits para una entrada con U/S en 0 */
#define PG_KERNEL 2

/* @brief Bits para una entrada con U/S en 0, presente*/
#define PG_KERNEL_PRESENT 3 

/* @brief Bits para una entrada con U/S en 1, presente */
#define PG_USER_PRESENT 7

/* @brief Las tablas de página se ubican en los últimos 4 MB de la memoria
 * virtual. Se usan 1023 tablas de página. La última tabla de páginas es el
 * mismo directorio de tablas de página.  */
#define KERNEL_PAGETABLES_VADDR 0xFFC00000

/** @brief El directorio de tablas de página se ubica como la última tabla de
 * páginas en la memoria virtual */
#define KERNEL_PD_VADDR (KERNEL_PAGETABLES_VADDR + 0x3FF000)

/** @brief Redondea una dirección dada al inicio de la página */
#define ROUND_DOWN_TO_PAGE(value) ((int)(value / PAGE_SIZE) * PAGE_SIZE)

/** @brief Redondea una dirección dada al inicio de la siguiente página */
#define ROUND_UP_TO_PAGE(value) (((int)(value / PAGE_SIZE) + 1) * PAGE_SIZE)

/** @brief Excepcion de fallo de pagina. */
#define PAGE_FAULT_EXCEPTION 14

/* No incluir de aqui en adelante si se incluye este archivo desde codigo
 * en ensamblador */
#ifndef ASM

/** @brief Entrada de una tabla de páginas */
typedef unsigned int page_table_entry;

/** @brief Entrada en el directorio de tablas de página */
typedef unsigned int page_directory_entry;

/** @brief Tipo de datos para el directorio de tablas de página */
typedef page_directory_entry * page_directory;

/** @brief Tipo de datos para la tabla de páginas */
typedef page_table_entry * page_table;

/** @brief Dirección física de directorio de tablas de página del kernel */
extern unsigned int kernel_pd_addr;

/** @brief Apuntador al inicio del directorio de tablas de página */
extern page_directory kernel_pd;

/** @brief Macro para invalidar una página en el TLB */
#define invalidate_page(addr) \
  inline_assembly ("invlpg (%0)" : : "a" (ROUND_DOWN_TO_PAGE(addr)))

/** @brief Completa el proceso de configurar la paginación para el kernel.  */
void setup_paging();

/** @brief Permite mapear una página a un marco de páginas en el espacio virtual
 * del kernel 
 */
int map_page(unsigned int vaddr, unsigned int addr);

/** @brief Permite quitar una página del espacio virtual */
int unmap_page(unsigned int vaddr);

/** @brief Quita una página del espacio virtual y libera el marco asociado */
int destroy_page(unsigned int vaddr);

/** @brief Manejador por defecto para fallo de página. */
void page_fault_handler(interrupt_state * state);

#endif

#endif /* PAGING_H_ */
