/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Este archivo implementa algunas rutinas para la gestion de direcciones
 * de memoria y cadenas de caracteres.
 */

#include <string.h>
#include <stdlib.h>

/**
 * @brief Copia un numero determinado de bytes
 */
void *memcpy(void *dst, const void *src, int count) {
	const char *sp = (const char *) src;
	char *dp = (char *)dst;
	for (;count != 0; count--) {
		*dp = *sp;
		dp++;
		sp++;
	}
	return dst;
}

/**
 * @brief Replica un valor (char) en un buffer.
 */
void *memset(void *dst, char val, int count) {
	char *tmp = (char *) dst;
	for (;count !=0; count--) {
		*tmp = val;
		tmp++;
	}
	return dst;
}

/**
 * @brief Calcula la longitud de una cadena.
 */
int strlen(const char *str){
	int ret;
	for (ret = 0; *str != '\0'; str++, ret++);
	return ret;
}

/**
 * @brief Copia los bytes de una cadena a otra.
 */
char * strcpy(char *dst, const char *src) {
	const char * x;
	char *y;
	int count;

	for (x=src, y=dst, count=0; *x != 0; y++,x++, count++){
		*y = *x;
	}
	*y=0; //Finalizar la segunda cadena
	return dst;
}

/**
 * @brief Concatena la cadena destino al final de la cadena fuente.
 */
char * strcat(char *dst, const char *src) {
	const char * x;
	char *y;

	/* Ir al final de la cadena de destino*/
	for (y=dst; *y != 0;y++);

	for (x=src; *x!= 0; x++, y++) {
		*y = *x;
	}

	/* Terminar la cadena de destino con nulo */
	*y = 0;

	return dst;
}

/**
 * @brief Compara dos cadenas de caracteres.
 */
int strcmp(const char *a, const char *b) {
	const char *x = a;
	const char *y = b;

	/* Avanzar hasta encontrar fin de alguna cadena o caracter diferente */
	for (x = a, y = b; *x != 0 && *y != 0 && *x == *y; x++, y++);

	/* Verificar si una cadena es mas corta que la otra*/
	if (*x == 0) {
		if (*y == 0) {
			return 0;
		}
		return -1;
	}else {
		if (*y == 0) {
			return 1;
		}
		return *y - *x;
	}
}

/**
 * @brief Compara los primeros n bytes de dos cadenas.
 */
int strncmp(const char *a, const char *b, int n) {
    for (; n > 0 && *a != 0 && *b != 0 && *a == *b; a++, b++, n--);

    if (n == 0) { 
        return 0;
    }else {
        return *b - *a;
    }
}

/**
 * @brief Busca c desde el inicio de str.
 */
char * strchr(const char *s, const char c) {

    if (s == 0 || *s == 0) {
        return 0;
    }

    while (*s != 0) {
        if (*s == (char)c) {
            return (char *)s;
        }
        s++;
    }
    return 0;
}

/**
 * @brief Busca c desde el final de s.
 */
char * strrchr(const char *s, const char c) {

    char * aux;
    if (s == 0 || *s == 0) {
        return 0;
    }

    aux = (char*)s;

    while (*s != 0) {
        s++;
    }

    while (s >= aux) {
        if (*s == (char)c) {
            return (char *)s;
        }
        s--;
    }
    return 0;
}


/**
 * @brief Permite obtener una 'palabra' de una cadena de entrada.
 */
int nexttok(char * source, char * destination, char delim, int offset) {
    char * src =  source;
    char * dst = destination;
    char c;
    int nchars;

    c=0;
    
    nchars = 0;

    if (src == 0) {
            return 0;
    }
    if (*src == 0) {
             return 0;
    }

    while ( (c=src[offset++]) !=0 && c == delim){ //Ignorar espacios antes
       nchars++;
    }

    if (c == 0) { //La cadena de entrada se termino!
          return 0;
    }else { //Regresar un caracter!
          offset--;
    }

    while ( (c=src[offset++]) != 0 && c != delim) { //Sacar el valor
          *dst++ = c;
          nchars++;
    }    

    if (nchars > 0) { //Terminar correctamente la cadena de destino
      *dst = 0;
    }

   return nchars;
}

/**
 * @brief Imprime datos en un buffer de salida.
 */
int sprintf(char * dst, char * format, ...) {
    char ** arg;
    char c;
    char buf[255];
    char *p;
    int i;

    char * aux;

    aux = dst;

    //Posicionar arg en la dirección de format
    arg = (char **)&format;

    /* Avanzar arg para que apunte al siguiente parametro */
    arg++;

    while ((c = *format++) != '\0') {
            //Buscar el indicador de formato '%'
            if (c != '%') { //Imprimir el caracter
                    *dst++ = c;
                    *dst = 0;
                    continue; //Pasar a la siguiente iteracion
            }
            //c = '%', el siguiente caracter indica el tipo de datos
            c = *format++;
            if (c == 'd') { //Entero con signo
                itoa (*((int *) arg++), buf, 10);
                strcat(dst, buf);
                dst += strlen(buf);
            }else if (c == 'u') { //Entero sin signo
                utoa (*((int *) arg++), buf, 10);
                strcat(dst, buf);
                dst += strlen(buf);
            }else if(c == 'x') { //hex
                itoa (*((int *) arg++), buf, 16);
                strcat(dst, buf);
                dst += strlen(buf);
            }else if(c == 'b') { //binario
                itoa (*((int *) arg++), buf, 2);
                strcat(dst, buf);
                dst += strlen(buf);
            }else if(c == 'o') { //octal
                itoa (*((int *) arg++), buf, 8);
                strcat(dst, buf);
                dst += strlen(buf);
            } else if(c == 's') { //String
                p = *arg++;
                if (p != 0 && *p != 0){
                    strcat(dst, p);
                    dst += strlen(p);
                }
            }else { //En caso contrario, mostrar la referencia
                *dst++ = *((int *) arg++);
                *dst = 0;
            }
    }
    return dst - aux;
}

