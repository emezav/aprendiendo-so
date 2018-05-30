/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 * @brief Contiene las estructuras relacionadas con la gestión de regiones
 * de memoria.
 */

#ifndef MEM_H_
#define MEM_H_
#include <bitmap.h>

/** @brief Descriptor de region de memoria */
typedef struct memory_region {
    /** @brief Apuntador a la siguiente region de memoria */
    struct memory_region * next;
    /** @brief Apuntador a la anterior region de memoria */
    struct memory_region * prev;
    /** @brief Direccion de inicio de la region */
    unsigned int start;
    /** @brief Tamaño de la region */
    unsigned int length;
    /** @brief Tipo de la region de memoria */
    bitmap map;
}memory_region;

#endif /* MEM_H_ */
