/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 * @brief Contiene las definiciones relacionadas con las gestión de memoria
 * virtual del espacio del kernel.
 */

#ifndef KMEM_H_
#define KMEM_H_
#include <mem.h>
#include <physmem.h>
#include <paging.h>

/** @brief Cantidad maxima de memoria virtual */
#define KMEM_LIMIT 0xFFFFFFFF

/** @brief Cantidad máxima de memoria del kernel */
#define KMEM_MAXSIZE (KMEM_LIMIT - KERNEL_VIRT_OFFSET)

/** @brief Memoria reservada al final del espacio virtual (16 MB) */
#define KMEM_RESERVED 0x1000000

/** @brief Máximo número de páginas en memoria virtual para el kernel */
#define KMEM_MAXPAGES (KMEM_MAXSIZE / PAGE_SIZE)

/** @brief Granularidad de la memoria virtual del kernel (16 MB) */
#define KMEM_GRANULARITY 0x1000000

/** @brief Cantidad de regiones de memoria de KMEM_GRANULARITY.*/
#define KMEM_REGION_COUNT (KMEM_MAXSIZE / KMEM_GRANULARITY)

/** @brief Paginas no contiguas */
#define KMEM_SPARSE 0

/** @brief Paginas contiguas */
#define KMEM_CONTIGUOUS 1 

/** @brief Numero total de paginas disponibles */
extern int kmem_available_pages;

/**
 * @brief Inicializa la memoria virtual del kernel
 */
void setup_kmem(void);

/**
 * @brief Busca y reserva una página libre dentro de la memoria del kernel
 * @return Dirección de inicio de la página
 */
unsigned int kmem_get_page(void);

/**
 * @brief Busca una región continua de páginas libres en la memoria del kernel
 * @return Dirección de inicio de la página
 */
unsigned int kmem_get_pages(int count);

/**
 @brief Busca una página y un marco libre y realiza el mapeo
 * @return Dirección de inicio de la página
 */
unsigned int kmem_allocate_page(void);

/**
 * @brief Busca y mapea una región continua de páginas libres
 * @param count Numero de paginas a buscar y mapear
 * @param sparse KMEM_SPARSE | KMEM_CONTIGUOUS  
 * @return Dirección de inicio de la página
 */
unsigned int kmem_allocate_pages(int count, int sparse);

/**
 * @brief Permite liberar una página
 * @param addr Dirección de la página a liberar
 * @return 1 si exitoso, 0 si error.
 */
int kmem_free(unsigned int addr);

/**
 * @brief Libera un conjunto de paginas contiguas
 * @param addr Dirección de la página primera página
 * @param count Número de páginas a liberar
 * @return 1 si exitoso, 0 si error.
 */
int kmem_free_pages(unsigned int start, unsigned int count);

/**
 * @brief Retorna el número de páginas disponibles en la memoria del kernel
 * @return Número de páginas disponibles.
 */
int available_pages();

#endif /* KMEM_H_ */
