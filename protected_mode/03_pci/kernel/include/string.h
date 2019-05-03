/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Contiene algunas definiciones para funciones sobre cadenas de
 * caracteres y regiones de memoria.
 */

#ifndef STRING_H_
#define STRING_H_

#define BUFSIZ 4096

/**
 * @brief Copia un numero determinado de bytes
 * de una posicion de memoria a otra.
 * @param dst Dirección de memoria de destino de los datos
 * @param src Dirección de memoria de fuente de los datos
 * @param count Numero de bytes a copiar
 * @return dirección de memoria de destino de los datos
 */
void *memcpy(void *dst, const void *src, int count);

/**
 * @brief Copia auna region de memoria un valor (char) determinado.
 * @param dst Dirección de memoria de destino
 * @param val Valor a copiar
 * @param count Numero de repeticiones
 * @return dirección de memoria de destino
 */
void *memset(void *dst, char val, int count);

/**
 * @brief Calcula la longitud de una cadena
 * terminada en el caracter nulo.
 * @param str Apuntador al inicio de la cadena de caracteres
 * @return Longitud de la cadena. No incluye en la longitud el caracter nulo
 *  al final de la cadena.
 *  */
int strlen(const char *str);

/**
 * @brief Copia los bytes de una cadena a otra.
 * @param dst Apuntador a la cadena destino
 * @param src Apuntador a la cadena fuente
 * @return Apuntador a la cadena destino
 */
char * strcpy(char *dst, const char *src);

/**
 * @brief Concatena la cadena destino al final de la cadena fuente.
 * @param dst Apuntador a la cadena de destinno
 * @param src Apuntador a la cadena fuente
 * @return Apuntador a la cadena de destino
 */
char * strcat(char *dst, const char *src);

/**
 * @brief Compara dos cadenas de caracteres.
 * @param a Apuntador a la cadena de destinno
 * @param b Apuntador a la cadena fuente
 * @return Resultado de la comparacion ( 0 :  cadenas iguales,
 *  		!= 0 : cadenas diferentes)
 */
int strcmp(const char *a, const char *b);

/**
 * @brief Compara los primeros n bytes de dos cadenas
 * @param a Apuntador a la cadena de destinno
 * @param b Apuntador a la cadena fuente
 * @param n Apuntador a la cadena fuente
 * @return Resultado de la comparacion ( 0 :  cadenas iguales,
 *  		!= 0 : cadenas diferentes)
 */
int strncmp(const char *a, const char *b, int n);

/**
 * @brief Busca c desde el inicio de str.
 * @param str Cadena de caracteres terminada en nulo
 * @param c Caracter a buscar.
 * @return Apuntador a la primera ocurrencia de c en s.
 */
char * strchr(const char *s, const char c);

/**
 * @brief Busca c desde el final de str.
 * @param str Cadena de caracteres terminada en nulo
 * @param c Caracter a buscar.
 * @return Apuntador a la primera ocurrencia de c en s.
 */
char * strrchr(const char *s, const char c);

/**
 * @brief Permite obtener una 'palabra' de una cadena de entrada.
 * @param source Cadena de entrada
 * @param destination Cadena en la cual se almacena la palabra obtenida
 * @param delim Caracter delimitador de palabras.
 * @param offset Desplazamiento inicial en la cadena de entrada
 * @return Numero de caracteres consumidos de la cadena de entrada
 * */
int nexttok(char * source, char * destination, char delim, int offset);

/**
 * @brief Imprime datos en un buffer de salida
 * @param dst Buffer de destino, que se termina en nulo
 */
int sprintf(char * dst, char * format, ...);

#endif /* STRING_H_ */
