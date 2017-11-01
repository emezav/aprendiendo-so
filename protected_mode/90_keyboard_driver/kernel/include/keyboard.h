/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 * @brief Contiene las definiciones para el manejador de teclado (8042)
 */

#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include <pm.h>

/** @brief Número de IRQ para el controlador de teclado */
#define KB_IRQ 1

/** @brief Puerto de estado y comandos del 8042 */
#define KB_STATUS_PORT 0x64

/** @brief Puerto de salida del 8042 */
#define KB_OUTPUT_PORT 0x60

/** @brief Numero de slots en el mapa de teclado */
#define KB_KEYS 128

/** @brief Bit activo cuando el codigo de escaneo es Break */
#define KB_BREAK 0x80

/** @brief Bit activo cuando hay datos para el sistema */
#define KB_HAS_OUTPUT 1

/** @brief Estado de las teclas modificadoras*/
typedef struct {
    char shift;
    char alt;
    char ctrl;
    char shiftlock;
}kb_mod_keys;

/** @brief Inicializa el driver de teclado */
void setup_keyboard();

/** @brief Rutina manejadora de la IRQ del teclado */
void keyboard_handler(interrupt_state * state);


/** @brief Obtiene un caracter del teclado */
char getchar();

/** 
 * @brief Obtiene una cadena de caracteres del teclado 
 * @param char * Apuntador al buffer para leer los datos
 * @param int Numero de caracteres a leer
 * @return Numero de caracteres leidos
 */
int gets(char * buf, int sz);
#endif /* KEYBOARD_H_ */

