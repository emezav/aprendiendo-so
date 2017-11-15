/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 * @brief Gestion de cache de objetos pequeños
 */
#include <stdlib.h>
#include <kcache.h>

/**
 * @brief Crea un cache de un tamaño determinado
 * @param Tamaño de los objetos en el cache
 * @return Referencia al nuevo cache
 */
kcache * kcache_create(int objsize) {
    return 0;
}

/** 
 * @brief Reservar un objeto dentro de un cache
 * @param Cache del cual reservar el objeto
 * @return Direccion del nuevo objeto, cero si no hay espacio*/
void * kcache_alloc(kcache * cache) {
    return 0;
}

/**
 * @brief Libera un objeto del cache
 * @param addr Direccion del objeto a liberar
 */
void kcache_free(void * addr){
    return;
}

