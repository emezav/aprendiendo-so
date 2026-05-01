/**
 * @file
 * @ingroup kernel_code
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License.
 *
 * @brief Definiciones publicas para el controlador de teclado.
 */

#ifndef KEYBOARD_H_
#define KEYBOARD_H_

/** @brief Puerto de datos del controlador de teclado 8042. */
#define KEYBOARD_DATA_PORT 0x60

/** @brief Puerto de estado del controlador de teclado 8042. */
#define KEYBOARD_STATUS_PORT 0x64

/** @brief Bit del registro de estado que indica si hay datos disponibles. */
#define KEYBOARD_OUTPUT_BUFFER_FULL 0x01

/** @brief Tamano del buffer circular de caracteres del teclado. */
#define KEYBOARD_BUFFER_SIZE 128

/**
 * @brief Inicializa el controlador de teclado.
 *
 * Instala el manejador de IRQ1 y deja preparado el modulo para recibir
 * scancodes del teclado PS/2.
 */
void setup_keyboard(void);

/**
 * @brief Obtiene el ultimo scancode recibido.
 * @return Ultimo scancode leido del controlador.
 */
unsigned char keyboard_get_last_scancode(void);

/**
 * @brief Obtiene el ultimo caracter ASCII decodificado.
 * @return Ultimo caracter ASCII, o cero si no hubo traduccion.
 */
char keyboard_get_last_ascii(void);

/**
 * @brief Determina si hay caracteres pendientes en el buffer del teclado.
 * @return 1 si hay al menos un caracter disponible, 0 en caso contrario.
 */
int keyboard_has_char(void);

/**
 * @brief Obtiene un caracter del buffer del teclado.
 * @return Caracter ASCII disponible, o cero si el buffer esta vacio.
 */
char keyboard_getchar(void);

#endif /* KEYBOARD_H_ */
