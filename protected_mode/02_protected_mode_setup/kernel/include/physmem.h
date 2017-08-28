/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 * @brief Contiene las definiciones relacionadas con las gestión * de memoria
 * física.
 */

#ifndef PHYSMEM_H_
#define PHYSMEM_H_
#include "bitmap.h"

/** @brief Granularidad de la memoria fisica */
#define PHYSMEM_GRANULARITY 0x1000000

#define PHYSMEM_REGION_COUNT (0xFFFFFFFF / PHYSMEM_GRANULARITY)

/** @brief Limite inferior de la memoria fisica  = 16 MB */
#define PHYSMEM_LOW_LIMIT 0x1000000

/** @brief Número de bytes que tiene una entrada en el mapa de bits */
#define BYTES_PER_ENTRY sizeof(unsigned int)

/* @brief Núnero de bits por entrada en el mapa de bits */
#define BITS_PER_ENTRY (8 * BYTES_PER_ENTRY)

/** @brief Entrada en el mapa de bits correspondiente a una dirección */
#define bitmap_entry(addr) \
	( addr /  PAGE_SIZE) / ( BITS_PER_ENTRY )

/** @brief Desplazamiento en bits dentro de la entrada en el mapa de bits */
#define bitmap_offset(addr) \
	(addr / PAGE_SIZE) % ( BITS_PER_ENTRY )

/** @brief Descriptor de region de memoria */
typedef struct memory_region {
    /** @brief Apuntador a la siguiente region de memoria */
    struct memory_region * next;
    /** @brief Apuntador a la anterior region de memoria */
    struct memory_region * prev;
    /** @brief Direccion de inicio de la region */
    unsigned int start;
    /** @brief Tamaño de la region */
    unsigned int length;
    /** @brief Tipo de la region de memoria */
    bitmap map;
}memory_region;


/**
 * @brief Inicializa el mapa de bits de memoria,
 * a partir de la informacion obtenida del GRUB.
 */
void setup_physical_memory(void);

/**
 @brief Busca un marco libre dentro del mapa de bits de memoria.
 * @return Dirección de inicio del marco.
 */
unsigned int allocate_frame(void);

/** @brief Busca una región de memoria contigua libre dentro del mapa de bits
 * de memoria.
 * @param length Tamaño de la región de memoria a asignar.
 * @return Dirección de inicio de la región en memoria.
 */
unsigned int allocate_frame_region(unsigned int length);

/**
 * @brief Permite liberar un marco de página
 * @param addr Dirección de inicio del marco. Se redondea hacia abajo si no es
 * múltiplo de PAGE_SIZE
 */
void free_frame(unsigned int addr);

#endif /* PHYSMEM_H_ */
