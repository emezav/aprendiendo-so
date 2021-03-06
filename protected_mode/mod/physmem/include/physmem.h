/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 * @brief Contiene las definiciones relacionadas con las gestión de memoria
 * física.
 */

#ifndef PHYSMEM_H_
#define PHYSMEM_H_
#include <mem.h>

/** @brief Tamaño en bytes de un marco de pagina */
#define FRAME_SIZE 4096

/** @brief Cantidad máxima de memoria física: 4 GB*/
#define PHYSMEM_MAXSIZE 0xFFFFFFFF

/** @brief Máximo número de páginas en memoria física (marcos de página)*/
#define PHYSMEM_MAXFRAMES (PHYSMEM_MAXSIZE / FRAME_SIZE)

/** @brief Granularidad de la memoria fisica */
#define PHYSMEM_GRANULARITY 0x1000000

#define PHYSMEM_REGION_COUNT (PHYSMEM_MAXSIZE / PHYSMEM_GRANULARITY)

/** @brief Limite inferior de la memoria fisica  = 16 MB */
#define PHYSMEM_LOW_LIMIT 0x1000000

/** @brief Redondea una dirección dada al inicio del marco */
#define ROUND_DOWN_TO_FRAME(value) ((int)(value / FRAME_SIZE) * FRAME_SIZE)

/** @brief Redondea una dirección dada al inicio del siguiente marco */
#define ROUND_UP_TO_FRAME(value) (((int)(value / FRAME_SIZE) + 1) * FRAME_SIZE)

/* @brief Numero total de marcos de pagina disponibles */
extern int physmem_available_frames;

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

/** 
 * @brief Busca una región de memoria contigua libre dentro del mapa de bits
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

/**
 * @brief Retorna el número de marcos de mágina libres
 * @return Número de marcos de página disponibles
 */
int available_frames();

#endif /* PHYSMEM_H_ */
