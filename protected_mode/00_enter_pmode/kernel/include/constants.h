/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Contiene algunas constantes necesarias para  el kernel */

/** @brief dirección fisica del kernel en memoria */
#define KERNADDR 0x1000 

/** @brief Segmento en el cual se carga el kernel.
 *
 * Este valor debe estar sincronizado con phys de link.ld */
#define KERNSEG KERNADDR >> 4

