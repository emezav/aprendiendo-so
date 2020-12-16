/**
* @file
* @ingroup kernel_code 
* @author Erwin Meza <emezav@gmail.com>
* @copyright GNU Public License. 
*
* @brief Gestión de almacenes de memoria
* Este módulo permite crear almacenes de memoria.
* Un almacén de memoria contiene uno o varios almacenes de bloques del tamaño especificado.
* Cuando el almacén se queda sin espacio, se adiciona automáticamente un nuevo
* almacén de bloques.
*/

#include <console.h>
#include <kmemstore.h>

/** @brief Almacen de bloques para los almacenes de memoria. */
kpool * kernel_memstore = 0;

/**
 * @brief Reserva o aumenta la capacidad de almacen de memoria.
 * Reserva la memoria requerida por el almacen.
 * Si el almacen es nuevo, reserva la memoria requerida.
 * Si  ya tiene memoria asignada, le adiciona un nuevo almacen de bloques. 
 * @param ms Almacen de memoria a inicializar.
 * @return 1 si se pudo reservar o aumentar la capacidad del almacen, 0 en caso contrario.
 */
int memstore_grow(kmemstore * ms);

/** @brief Crea e inicializa un almacen de memoria. 
 * El almacen ocupa una pagina para blocksize <= PAGE_SIZE y la cantidad
 * de paginas necesarias para un bloque si blocksize > PAGE_SIZE
 * @param blocksize Tamaño de cada bloque de memoria
 * @return Nuevo almacen de memoria, dentro del cual se ha inicializado un almacen de bloques.
*/
kmemstore * new_memstore(unsigned int blocksize) {

  /* Obtener la referencia a un nuevo almacen de memoria. */
   kmemstore * ret = kpool_alloc(kernel_memstore);

  /* Si no hay espacio en el almacen del kernel, crear un nuevo almacen */
  if (ret == 0) {

    /* Crear un nuevo almacen de bloques (para obtener un nuevo almacen de memoria)*/
    kpool * new_ms_pool = new_kpool();
    if (new_ms_pool == 0) {
      return 0;
    }

    //Reservar una pagina para guardar los almacenes
    unsigned char * mem = (unsigned char*) kmem_allocate_page();

    kpool_init(new_ms_pool, mem, sizeof(kmemstore), (PAGE_SIZE / sizeof(kmemstore)));

    /* Adicionar el nuevo almacen al inicio del la lista de almacenes reemplazar la cabeza de la lista. */
    kernel_memstore = kpool_add(kernel_memstore, new_ms_pool);
    ret =  kpool_alloc(kernel_memstore);
  }  

  /* No se pudo obtener o crear el almacen de memoria. */
  if (ret == 0) {
      return 0;
  }

  /* Inicializar el nuevo almacen. */
  ret->blocksize = blocksize;
  ret->count  = 0; //por inicializar
  ret->free = 0; //por inicializar
  ret->pool = 0; //por inicializar

  if (!memstore_grow(ret)) {
    console_printf("No se pudo reservar espacio para el almacen de memoria \n");
    kpool_free(kernel_memstore, ret);
    return 0;
  }
  
  return ret;
}

/** 
* @brief Reserva un bloque de memoria en un almacén.
* @param ms Almacén de memoria.
* @return Referencia al nuevo bloque, 0 si no se puede reservar.
*/
void * memstore_alloc(kmemstore * ms) {

  void * ptr = kpool_alloc(ms->pool);

  /* Si se obtiene un apuntador valido, decrementar la cantidad de bloques libres
   y retornar el apuntador. */
  if (ptr != 0) {
    ms->free--;
    return ptr;
  }

  /* Tratar de aumentar la capacidad del almacen de memoria*/
  if (!memstore_grow(ms)) {
    return 0;
  }

  /* Tratar de nuevo de reservar memoria. */
  ptr = kpool_alloc(ms->pool);

  /* Si se obtiene un nuevo bloque de memoria, decrementar la cantidad de bloques libres. */
  if (ptr != 0) {
    ms->free--;
  }

  return ptr;

}


int  memstore_free(kmemstore * ms, void * ptr) {
  if (kpool_free(ms->pool, ptr)) {
    ms->free++;
    return 1;
  }
  return 0;
}

/**
 * @brief Reserva nueva memoria o aumenta el espacio un almacen de memoria. */
int memstore_grow(kmemstore * ms) {
  if (ms == 0) {
    return 0; //???
  }

  /* Obtener un nuevo almacen de bloques */
  kpool * pool = new_kpool();

  if (pool == 0) {
      console_printf("No se pudo crear el almacen de bloques\n");
      return 0;
  }

  int blocksize = ms->blocksize;

  int pages;
  int count;

  /* Determinar la cantidad de paginas que necesita el almacen */
  if (blocksize <= PAGE_SIZE) {
    count = PAGE_SIZE / blocksize;
    pages = 1;
  }else {
    pages = blocksize / PAGE_SIZE;
    if (blocksize % PAGE_SIZE != 0) {
      pages++;
    }    
    count = (pages * PAGE_SIZE) / blocksize;
  }

  /* Obtener las paginas de memoria para el almacen*/
  unsigned char * ptr = (unsigned char *)kmem_allocate_pages(pages, KMEM_SPARSE);
  if (ptr == 0) {
    delete_kpool(pool);
    console_printf("No se pudieron obtener %d paginas del kernel para el almacen\n", pages);
    return 0;
  }

  /* Inicializar el nuevo almacen de bloques */
  kpool_init(pool, ptr, blocksize, count);

  /* Asociar el almacen de bloques al almacen de memoria*/
  if (ms->pool == 0) {
    /* El almacen es nuevo, solo contiene el almacen de bloques que se acaba de crear.*/
    ms->count = count;
    ms->free = count;
    ms->pool = pool;
  }else {
    /* Adicionar el almacen de bloques que se acaba de crear. */
    ms->count += count;
    ms->free += count;
    ms->pool = kpool_add(ms->pool, pool);
  }

  return 1;
}