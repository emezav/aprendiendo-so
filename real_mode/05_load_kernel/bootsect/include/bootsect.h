/**
 * @file
 * @ingroup bootsector_code
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright MIT License.
 *
 * @brief Contiene constantes necesarias para  el cargador de arranque */

/** 
* Geometría por defecto del floppy: 80 cilindros, 2 cabezas, 18 sectores por
* pista (SPT).  Total sectores: 2880 Se utilizan las constantes HEADS y
* SPT para calcular el cilindro, la cabeza y el sector que corresponden a un
* sector LBA 'i' especificado.  
*/

/** Cilindros que tiene un floppy */
#define CYLS 80

/** Cabezas que tiene un floppy*/
#define HEADS 2

/** Sectores por pista de un floppy */
#define SPT 18

