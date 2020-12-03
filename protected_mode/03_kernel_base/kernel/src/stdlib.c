/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Contiene las implementaciones de algunas funciones de utilidad
 */

#include <stdlib.h>

/**
 * @brief Convierte un numero en base 2, 10 0 16 a un string terminado
 * en nulo. Si la base es 10, toma el numero con signo.
 *  @param n Número a transformar en cadena
 *  @param buf Buffer que contiene el número transformado a cadena de caracteres
 *  @param base Base a la cual se desea transformar el número (2, 10 o 16).
 *  @return  Apuntador al buffer en el cual se encuentra el número transformado
 */
char * itoa(unsigned int n, char *buf, int base) {
	    unsigned int remainder;
        unsigned int number;
        int is_negative;

        char *p, *r;
        char c;

        if (base <=1 || base > 16 ) { //Evitar ciclo infinito o desbordamiento!
                *buf='\0';
                return buf;
        }

        p = buf;
        number = n;
        is_negative = 0;

        //Si el numero es negativo y la base es 10, colocar el caracter '-'
        if ( base == 10 && test_bit(number, 31)) {
				number = (unsigned int)-number;
				is_negative = 1;
                *p = '-';
                p++;
        }

        do {
       	/* remainder = number mod base: Sacar el digito menos significativo */
			remainder = number % base;
			if (remainder <= 9) {
				*p = '0' + remainder;
			}else {
				*p = 'a' + (remainder - 10);
			}

			p++;	/* Aumentar el apuntador de la cadena */
			number = number / base; /* Disminuir el numero */
        }while (number > 0);

        *p = 0; //Cadena terminada en nulo

        /* Debidio a que Intel es una arquitectura little-endian,
         * el string que se encontro esta invertido.
         * Algoritmo: Establecer un apuntador al final de la cadena, y un
         * apuntador al inicio. Intercambiar el valor al que apuntan hasta
          llegar a la mitad de la cadena.*/
        r = p - 1; //El ultimo caracter es nulo, ir hacia atras un caracter
        p = buf;

		if ( is_negative ) { /* Numero negativo? */
			p++;
		}

		/* Mientras no se haya llegado a la mitad, intercambiar caracteres */
        while (p < r) {
                c = *r;
                *r = *p;
                *p = c;
                p++;
                r--;
        }
        return buf;
}

/**
 * @brief Convierte un numero sin signo en base 2, 10 0 16 a un string terminado
 * en nulo. Si la base es 10, toma el numero con signo.
 *  @param n Número a transformar en cadena
 *  @param buf Buffer que contiene el número transformado a cadena de caracteres
 *  @param base Base a la cual se desea transformar el número (2, 10 o 16).
 *  @return  Apuntador al buffer en el cual se encuentra el número transformado
 */
char * utoa(unsigned int n, char *buf, int base) {
	    unsigned int remainder;
        unsigned int number;

        char *p, *r;
        char c;

        if (base <=1 || base > 16 ) { //Evitar ciclo infinito o desbordamiento!
                *buf='\0';
                return buf;
        }

        if (base != 10) { // Invocar a itoa, si la base es diferente de 10
        	return itoa(n, buf, base);
        }

        p = buf;
        number = n;

        do {
       	/* remainder = number mod base: Sacar el digito menos significativo */
			remainder = number % base;
			if (remainder <= 9) {
				*p = '0' + remainder;
			}else {
				*p = 'a' + (remainder - 10);
			}
			p++;	/* Aumentar el apuntador de la cadena */
			number = number / base; /* Disminuir el numero */
        }while (number > 0);

        *p = 0; //Cadena terminada en nulo

        /* Debidio a que Intel es una arquitectura little-endian,
         * el string que se encontro esta invertido.
         * Algoritmo: Establecer un apuntador al final de la cadena, y un
         * apuntador al inicio. Intercambiar el valor al que apuntan hasta
          llegar a la mitad de la cadena.*/
        r = p - 1; //El ultimo caracter es nulo, ir hacia atras un caracter
        p = buf;

		/* Mientras no se haya llegado a la mitad, intercambiar caracteres */
        while (p < r) {
                c = *r;
                *r = *p;
                *p = c;
                p++;
                r--;
        }
        return buf;
}

/**
 * @brief Convierte un string a un entero, en la base especificada
 *  @param buf Buffer que contiene el numero
 * 	@param base Base en la cual se quiere transformar el numero
 *  @return Número en la base especificada.
 */
int atoi(char *buf, int base) {

	char * p;
	char *q;
	char c;
	int is_negative;
	int started;
	int finished;
	int result;
	int multiplier;

	result = 0;

	if (buf == 0) {return 0;}

	if (base != 2 && base != 10 && base > 16) {
		return result;
	}

	/* Apuntador al inicio de la cadena de entrada */
	p = buf;

	c = 0;
	//Ignorar los espacios al inicio de la cadena
	while ((c = *p) != '\0' && c == ' ') {
		p++;
	}

	if (c == 0) { //No se encontro ningun caracter diferente a espacio!
		return result;
	}


	/* Escanear la cadena para buscar digitos validos en la base especificada,
	comenzando con en el primer caracter diferente de espacio */

	q = p;
	is_negative = 0;
	started = 0;
	finished = 0;

	while ((c = *q) && c != '\0' && !finished) {
		if (base == 2) {
			if (c < '0' || c > '1' ) {
				finished = 1;
				break;
			}else {
				q++;
			}
		}else if (base == 10) {
			if (c == '-') {
				if (started == 0) { /* No se ha encontrado ningun digito? */
					started = 1;
					is_negative = 1;
					p++;
					q++;
				}else { /* Termino el numero */
					finished = 1;
					break;
				}
			}else if (c < '0' || c > '9') {
				finished = 1;
			}else { //Caracter valido
				if (started == 0) {
					started = 1;
				}
				q++;
			}
		}else  { /* base != 2 y base != 10 */
			if ((c < '0' || c > '9') && (c < 'A' || c > ('A' - 10 + base)) &&
					(c < 'a' || c > ('a' - 10 + base))) {
				finished = 1;
				break;
			}else { //Caracter valido
				q++;
			}
		}
	}

	/* Si no se encontro ningun digito valido, retornar */
	if (q <= p) {
		return result;
	}

	/* Ahora construir el numero, desde el digito menos significativo
	 * hacia el digito mas siginificativo (de atras hacia adelante en la
	 * cadena de entrada)
	 * El algoritmo termina al llegar al inicio de la cadena
	 * */

	multiplier = 1;
	do {
		c=*--q; /* q apunta a el ultimo caracter valido + 1*/
		if (base == 2 || base == 10) {
			result = result + ( (c - '0') * multiplier);
			multiplier = multiplier * base;
		}else {
			if (c >= '0' && c <= '9') {
				result = result + ( (c - '0') * multiplier);
			}else if (c >= 'a' && c <= ('a' - 10 + base)) {
				result = result + ( (10 + (c - 'a')) * multiplier);
			}else if (c >= 'A' && c <= ('A' - 10 + base)) {
				result = result + ( (10 + (c - 'A')) * multiplier);
			}
			multiplier = multiplier * base;
		}
	}while (q != p);

	if (base == 10 && is_negative ==1) {
		result = -result;
	}
	return result;
}
