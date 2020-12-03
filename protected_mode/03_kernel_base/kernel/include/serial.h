/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Contiene las primitivas basicas para entrada / salida por
 * el puerto serial COM1
 */

#ifndef SERIAL_H_
#define SERIAL_H_

/** @brief Direccion de E/S del puerto COM1*/
#define COM1_PORT 0x3f8   /* COM1 */

/**
 * @brief Inicializa el puerto serial COM1
 */
void setup_serial();

/**
 * @brief Escribe un caracter en el puerto serial
 * @param c caracter a escribir
*/
void serial_putchar(char c);

/**
 * @brief Escribe una cadena de caracteres en el puerto serial
 * @param s cadena a escribir
 */
void serial_puts(char * s);

/**
 * @brief  Esa funcion implementa en forma basica el comportamiento de
 * 'printf' en C.
 * @param format Formato de la cadena de salida
 * @param ...  Lista de referencias a memoria de las variables a imprimir
 *
*/
void serial_printf(char * format,...);


#endif /* SERIAL_H_ */
