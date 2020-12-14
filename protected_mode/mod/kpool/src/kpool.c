/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Gestión de bloques de memoria
 */

#include <kpool.h>

/** @brief Inicializa un almacen de bloques. */
int kpool_init(kpool * p, 
               unsigned char * pool,
               unsigned int blocksize, 
               unsigned int count) {
  p->pool = pool;
  p->blocksize = blocksize;
  p->count = count;
  p->free = count;
  p->initialized = 0;
  p->freeptr = pool;
  return 0;
}

/** @brief Reserva un bloque de memoria en un almacén. */
void * kpool_alloc(kpool * p) {
  
  /* Retornar si no existen bloques libres*/
  if (p->free == 0) {
    return 0;
  }
  
  //Inicializar el siguiente bloque disponible
  if (p->initialized < p->count) {
    //Obtener un apuntador a la primera posicion sin inicializar
    unsigned int * ptr = (unsigned int*)(p->pool + (p->initialized * p->blocksize));
    //Incrementar la cantidad de bloques inicializados
    p->initialized++;
    //Inicializar la posicion actual
    *ptr = p->initialized;
  }
  
  //Apuntar al primer elemento disponible 
  void * ret = (void*)p->freeptr;
  
  //Decrementar la cantidad de bloques libres
  p->free--;
  
  //Si quedan bloques disponibles, actualizar freeptr
  if (p->free > 0) {
    //Quedan bloques libres
    p->freeptr = (unsigned char*)(p->pool + ((*(unsigned int*)p->freeptr) * p->blocksize));
  }else {
    //No quedan bloques libres
    p->freeptr = 0;
  }
  
  return ret;
}

/** @brief Libera un bloque de memoria de un almacén. */
void kpool_free(kpool * p, void * ptr) {
  
  //Verificar si el apuntador se encuentra en este pool
  if (ptr < (void *)p->pool 
      || ptr > (void *)(p->pool + (p->count * p->blocksize))) {
    return;
  }

  //Asegurar un desplazamiento de bloque en el apuntador recibido
  unsigned int offset = ((unsigned char *)ptr - p->pool) / p->blocksize;
  ptr = (void *)(p->pool + (offset * p->blocksize));
      

  if (p->freeptr != 0) {
    //Obtener el bloque libre actual
    unsigned int block_index = ((unsigned char *)p->freeptr - p->pool) / p->blocksize;
    
    //En este bloque recien liberado, almacenar el indice del bloque anterior libre
    *(unsigned int*)ptr = block_index;
    
    //Actualizar la cabeza de la lista de bloques libres a este bloque
    p->freeptr = (unsigned char *) ptr;
  }else {
    //No hay mas bloques libres, tomar este la cabeza de la lista
    p->freeptr = (unsigned char *) ptr;
  }
  
  //Incrementar la cantidad de bloques libres
  p->free++;
}
