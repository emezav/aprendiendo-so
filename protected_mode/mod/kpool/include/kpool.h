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


typedef struct kpool {
  unsigned int blocksize; // Tamaño del bloque
  unsigned int count; // Cantidad de bloques
  unsigned int initialized; // Cantidad de bloques inicializados
  unsigned int free; // Cantidad de bloques libres
  unsigned char * freeptr; //Apuntador al siguiente bloque libre
  unsigned char * pool; // Region de memoria para almacenar los bloques
  struct kpool * next; // Apuntador al siguiente almacen
} kpool;


/** 
 * @brief Obtiene un almacen desde el almacen central del kernel.
 * Esta rutina crea o extiende el almacen principal de ser necesario.
 * @return nuevo almacen de bloques listo para inicializar.
 */
kpool * new_kpool();

/** 
 * @brief Borra un almacen desde el almacen central del kernel.
 * @param p Almacen a eliminar.
 */
void delete_kpool(kpool * p);

/** 
* @brief Inicializa un almacen de bloques.
* @param p Referencia al almacen de bloques.
* @param pool Memoria para el almacén.
* @param blocksize Tamaño del bloque.
* @param count cantidad de bloques que contiene el almacén.
* @return Referencia al almacen inicializado.
*/
kpool * kpool_init(kpool * p, 
               unsigned char * pool,
               unsigned int blocksize, 
               unsigned int count);


kpool * kpool_add(kpool * p, kpool * new_p); 

/** 
* @brief Reserva un bloque de memoria en un almacén.
* @param p Almacén de bloques.
* @return Referencia al nuevo bloque, 0 si no se puede reservar.
*/
void * kpool_alloc(kpool * p);

/** 
* @brief Libera un bloque de memoria de un almacén.
* @param p Almacén de bloques.
* @param ptr Referencia al bloque que se desea liberar.
*/
int kpool_free(kpool * p, void * ptr);

#endif /* KPOOL_H_ */
