/**
* @file
* @ingroup kernel_code 
* @author Erwin Meza <emezav@gmail.com>
* @copyright GNU Public License. 
* @brief Almacenes de memoria.
*/

#ifndef KMEMSTORE_H_
#define KMEMSTORE_H_

#include <console.h>
#include <kmem.h>
#include <kpool.h>

typedef struct {
  unsigned int blocksize; // Tamaño del bloque
  unsigned int count; // Cantidad de bloques
  unsigned int free; // Cantidad de bloques libres
  struct kpool * pool; // Almacen de bloques
} kmemstore;

/** 
* @brief Inicializa un almacen de memoria.
* @param blocksize Tamaño del bloque.
* @return Referencia al almacen inicializado.
*/
kmemstore * new_memstore(
               unsigned int blocksize);

/** 
* @brief Reserva un bloque de memoria en un almacén.
* @param ms Almacén de memoria.
* @return Referencia al nuevo bloque, 0 si no se puede reservar.
*/
void * memstore_alloc(kmemstore * ms);

/** 
* @brief Libera un bloque de memoria de un almacén.
* @param ms Almacén de bloques.
* @param ptr Referencia al bloque que se desea liberar
* @return 1 si se pudo liberar la memoria, 0 en caso contrario.
*/
int memstore_free(kmemstore * ms, void * ptr);


/**
 * @brief Libera los almacenes bloques no usados. 
 * @param ms Almacen de memoria.
 */
void memstore_shrink(kmemstore * ms);

#endif /* KPOOL_H_ */
