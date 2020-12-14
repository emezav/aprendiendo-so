/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 * @brief Gestión de bloques de memoria
 * Implementación del algoritmo propuesto en:: 
 *  Fast Efficient Fixed-Size Memory Pool - No Loops and No Overhead. Ben Kenwright 2012.
 */

#ifndef KPOOL_H_
#define KPOOL_H_

typedef struct {
  unsigned int blocksize;
  unsigned int count;
  unsigned int initialized;
  unsigned int free;
  unsigned char * freeptr;
  unsigned char * pool;
} kpool;

/** 
 * @brief Inicializa un almacen de bloques.
 * @param p Referencia al almacen de bloques.
 * @param pool Memoria para el almacén.
 * @param blocksize Tamaño del bloque.
 * @param cantidad de bloques que contiene el almacén.
 * @return 0
 */
int kpool_init(kpool * p, 
               unsigned char * pool,
               unsigned int blocksize, 
               unsigned int count);

/** 
 * @brief Reserva un bloque de memoria en un almacén.
 * @param p Almacén de memoria.
 * @return Referencia al nuevo bloque, 0 si no se puede reservar.
 */
void * kpool_alloc(kpool * p);

/** 
 * @brief Libera un bloque de memoria de un almacén.
 * @param p Almacén de memoria.
 * @param ptr Referencia al bloque que se desea liberar.
 */
void kpool_free(kpool * p, void * ptr);

#endif /* KPOOL_H_ */
