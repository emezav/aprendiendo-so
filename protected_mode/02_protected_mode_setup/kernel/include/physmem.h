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

/**
 * @brief Marca una región de memoria como disponible
 * @param start_addr Dirección de memoria del inicio de la región a marcar como
 * disponible. Se redondea por debajo a un múltiplo de PAGE_SIZE
 * @param length Tamaño de la región en bytes, múltiplo de PAGE_SIZE
 */
void mark_available_memory(unsigned int start_addr, unsigned int length);

#endif /* PHYSMEM_H_ */
