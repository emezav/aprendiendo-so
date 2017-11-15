/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 * @brief Cache de objetos pequeños
 */

#ifndef KCACHE_H
#define KCACHE_H
#include <kmem.h>

#define KCACHE_MINSIZE 8
#define KCACHE_MAXSIZE 256

typedef struct {   
    int free_count;
    int object_size;
}kcache;

/**
 * @brief Crea un cache de un tamaño determinado
 * @param Tamaño de los objetos en el cache
 * @return Referencia al nuevo cache
 */
kcache * kcache_create(int objsize);

/** 
 * @brief Reservar un objeto dentro de un cache
 * @param Cache del cual reservar el objeto
 * @return Direccion del nuevo objeto, cero si no hay espacio*/
void * kcache_alloc(kcache * cache);

/**
 * @brief Libera un objeto del cache
 * @param addr Direccion del objeto a liberar
 */
void kcache_free(void * addr);


#endif /* KCACHE_H_ */
