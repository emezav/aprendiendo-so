/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Codigo de inicializacion del kernel en C
 *
 * Este codigo recibe el control de start.S y continúa con la ejecución.
 *
 * */

/**
 * @brief Recibe el control de start.S,
 * @param numsectors  número de bytes que ocupa el kernel
 * @param message mensaje de bienvenida del archivo start.S
 * */
void cmain(int numsectors, char * message) {
	/* Apuntador al inicio de la memoria de video */
	unsigned short * videoptr = (unsigned short *) 0xB8000;
	/* Caracter a imprimir en la memoria de video (atributos << 8 | ascii) */
	unsigned short c;

	int i, j;

	char * aux;

	char a, b, attr;

	aux = message;

	/* Imprimir la cadena, caracter por caracter */
	while (*aux != 0 ){
	/* Un caracter en la memoria de video es un word: (atributos << 8 | ascii)*/
		c = (0x1f << 8 | *aux);
		*videoptr = c;
		videoptr++; /* Avanzar en la memoria de video */
		aux++; /* Avanzar en la cadena */
	}

	/* Ahora imprimir un caracter por cada sector del kernel. */
	for (i=0; i< numsectors; i++) {
		c = (0x1f << 8 | '#');
		*videoptr = c;
		videoptr++;
	}

	/* Ahora combinar los atributos de texto y fondo para imprimir en la
	 * memoria de video de modo texto */
	for (a=0; a<16; a++){
		for (b=0; b<16; b++) {
			attr = (a << 4 | b) & 0xFF;
			c = (attr << 8 | '*');
			*videoptr = c;
			videoptr++;
		}
	}
}

