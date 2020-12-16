/**
* @file
* @ingroup kernel_code 
* @author Erwin Meza <emezav@gmail.com>
* @copyright GNU Public License. 
*
* @brief Gestión de bloques de memoria
*/

#include <console.h>
#include <kmem.h>
#include <kpool.h>

static inline int kpool_contains(kpool * p, void * ptr)
{
  return (ptr >= (void *)p->pool 
      && ptr < (void *)(p->pool + (p->count * p->blocksize)));
}

kpool * kernel_pool = 0;

/** 
 * @brief Crea un nuevo almacen de almacenes para el kernel.
 * Esta rutina obtiene una pagina del kernel, almacena la estructura de datos
 * del almacen al inicio de la pagina y configura el almacen justo despues
 * de la estructura de datos.
 * @return Nuevo almacen de almacenes.
*/
kpool * allocate_kpool_memory();

/** @brief Obtiene un almacen desde el almacen central del kernel. */
kpool * new_kpool() {
  kpool * ret = kpool_alloc(kernel_pool);

  /* Si no hay espacio en el almacen de bloques del kernel, crear un nuevo almacen */
  if (ret == 0) {
    kpool * new_p = allocate_kpool_memory();
    if (new_p == 0) {
      return 0;
    }
    /* Adicionar el nuevo almacen al inicio del la lista de almacenes reemplazar la cabeza de la lista. */
    kernel_pool = kpool_add(kernel_pool, new_p);
    ret =  kpool_alloc(kernel_pool);
  }  

  return ret;
}

/** @brief Borra un almacen desde el almacen central del kernel. */
void delete_kpool(kpool * p) {
  kpool_free(kernel_pool, p);
}


/** @brief Crea un nuevo almacen de almacenes para el kernel. */
kpool * allocate_kpool_memory() {
    /* Obtener una nueva pagina del kernel. La estructura de datos para el almacen se ubicara al inicio de la pagina. */
    kpool * new_pool = (kpool*)kmem_allocate_page();

    if (new_pool == 0) {
      return 0;
    }

    /* Los datos del almacen se almacenan en la misma pagina, despues de la estructura de datos */
    unsigned char * mem = (unsigned char*)new_pool + sizeof(kpool);

    unsigned int blocksize = sizeof(kpool);

    /* Cantidad de estructuras de datos que caben en una pagina (-1 porque la E.D. se almacena al inicio) */
    unsigned int count = ((PAGE_SIZE - sizeof(kpool))/ blocksize);

    kpool_init(new_pool, mem, blocksize, count);

    return new_pool;
}


/** @brief Inicializa un almacen de bloques. */
kpool * kpool_init(kpool * p, 
               unsigned char * pool,
               unsigned int blocksize, 
               unsigned int count) 
{
  p->pool = pool;
  p->blocksize = blocksize;
  p->count = count;
  p->free = count;
  p->initialized = 0;
  p->freeptr = pool;
  p->next = 0;
  return p;
}

kpool * kpool_add(kpool * p, kpool * new_p) {
  new_p->next = p;
  return new_p;
}

/** @brief Reserva un bloque de memoria en un almacén. */
void * kpool_alloc(kpool * p) {
  
  kpool * aux = p;
  
 /* Buscar el primer almacen con bloques libres */
 while (aux != 0 && aux->free == 0) {
   aux = aux->next;
 }

 //Si no hay almacenes con espacio, retornar.
 if (aux == 0) {
   return 0;
 }
 
 //Tomar el apuntador del almacen de bloques que tiene espacio
 p = aux;
 
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
int kpool_free(kpool * p, void * ptr) {
 
  kpool * aux = p;
  
  //Buscar el almacen que contiene este bloque
  while (aux != 0 && !kpool_contains(aux, ptr)) {
    aux = aux->next;
  }
  
  //Si el bloque no se encuentra en ningun almacen, retornar.
  if (aux == 0) {
    return 0;
  }
  
  p = aux;
  
  //Asegurar un desplazamiento de bloque en el apuntador recibidoz
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

  return 1;
}

