/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 * @brief Gestion de mapas de bits
 * física.
 */
 
#ifndef BITMAP_H
#define BITMAP_H

#define BITS_PER_BITMAP_ENTRY (8 * sizeof(unsigned int))
#define BITMAP_SET(dst, entry, offset) dst->data[entry] |= (1<<offset)
#define BITMAP_CLEAR(dst, entry, offset) dst->data[entry] &= ~(1<<offset)
#define BITMAP_TEST(dst, entry, offset) (dst->data[entry] & (1<<offset))

/** @brief Descriptor de mapa de bits */
typedef struct {	
  /** @brief apuntador al mapa de bits */
	unsigned int * data; 
  /** @brief Bits totales del mapa */
	int total_slots; 
  /** @brief Entradas totales del mapa */
	int total_entries;
  /** @brief Ultimo bit marcado como libre */
	int last_free;
  /** @brief Numero de bits libres */
	int free_slots;
}bitmap;

/** @brief Inicializa un mapa de bits.
 *  @param bitmap * Apuntador al descriptor de mapa de bits  
 *  @param unsigned int * Region de memoria del mapa de bits
 *  @param int total de bits
 *  @return 0 
 */
int bitmap_init(bitmap * dst, 
									unsigned int * data, 
									int total_bits);
                  
/* @brief Verifica el valor de un bit en el mapa de bits
 * @param bitmap * Apuntador al descriptor del mapa de bits
 * @param int Posicion del bit a verificar
 * @return int 0 o 1, dependiendo del valor de bit
 */
int bitmap_test(bitmap * dst, int slot);


/* @brief Busca y limpia un bit disponible en el mapa de bits
 * @param bitmap * Apuntador al descriptor del mapa de bits
 * @return Posicion del bit en el mapa de bits, -1 si no hay disponible
 */
int bitmap_allocate(bitmap * dst);

/* @brief Busca y limpia una region bit en el mapa de bits
 * @param bitmap * Apuntador al descriptor del mapa de bits
 * @param int Numero de bits continuos a buscar
 * @return Posicion del primer bit de la region, -1 si no hay dispnible 
 */
int bitmap_allocate_region(bitmap * dst, int count);

/* @brief Marca un bit como disponible en el mapa de bits 
 * @param bitmap * Apuntador al descriptor del mapa de bits
 * @param int Posicion del bit a liberar
 * @return 0 si exitoso, -1 si error 
 */
int bitmap_free(bitmap * dst, int slot);

/* @brief Marca una region como disponible en el mapa de bits 
 * @param bitmap * Apuntador al descriptor del mapa de bits
 * @param int Posicion del primer bit de la region a liberar
 * @return 0 si exitoso, -1 si error 
 */
int bitmap_free_region(bitmap * dst, int slot, int count);

#endif
