/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Contiene las primitivas basicas para entrada / salida
 */

#include <pm.h>
#include <asm.h>
#include <stdlib.h>
#include <console.h>

/** @brief Apuntador al inicio de la memoria de video.
 * @details
 * La memoria de video se encuentra mapeada en la dirección lineal 0xB8000.
 * Cada caracter en pantalla ocupa dos caracteres (bytes) en la memoria de
 * video:
 * - El byte menos significativo contiene el caracter ASCII a mostrar
 * - El byte más significativo contiene los atributos de texto y fondo
 *   del caracter a mostrar. A su vez este byte se subdivide en:
 *   @verbatim
 *    7  6  5  4  3  2  1  0
 *   +-----------------------+
 *   |I |F |F |F |I |B |B |B |
 *   +-----------------------+
 *   @endverbatim
 *   Los bits F correspondel al color del texto (Foreground).
 *   Los bits B corresponden al color de fondo (Background).
 *   El bit I corresponde a la intensidad del color de fondo (0 = oscuro,
 *   1 = claro) o del color del texto.
 */
unsigned short * videoptr = 
        (unsigned short *) (VIDEO_START_ADDR);

/** @brief Byte que almacena los atributos de texto */
char text_attributes = COLOR(LIGHTGRAY, BLACK);

/** @brief Variable que controla el número de líneas de la pantalla */
int screen_lines = SCREEN_LINES;

/** @brief Variable que controla el número de columnas de la pantalla */
int screen_columns = SCREEN_COLUMNS;

/** @brief Variable que controla la línea actual en la pantalla */
int current_line = 0;

/** @brief Variable que controla la columna actual en la pantalla */
int current_column = 0;

/**
 * @brief Función privada para subir una línea si se ha llegado al final
 * de la pantalla
 */
void console_scroll(void);

/**
 * @brief Función privada que permite actualizar el cursor en la pantalla.
 * */
void console_update_cursor(void);

/**
 * @brief Función para imprimir un caracter
 *
 * Imprime directamente en la memoria de video. Valida
 * caracteres especiales, como fin de línea, tabulador y backspace.
 * @param c caracter ascii a imprimir
 */
void console_putchar(char c) {

	int is_printable;

	unsigned int offset;
	unsigned short * tmp;
	char linea[80];

	/* El caracter es imprimible? */
	if (c >= ' ') {
		is_printable = 1;
	}else {
		is_printable = 0;
	}

	if (c == BACKSPACE) { /* Retroceder el apuntador de la memoria de video */
		if (current_column != 0) { //Ultima columna?
			current_column--;
		}else {
			if (current_line > 0) {
				current_column = SCREEN_COLUMNS;
				current_line--;
			}
		}
	}else if (c == TAB) { /* Mover TABSIZE caracteres */
		current_column = (current_column + TABSIZE) & ~(TABSIZE-1);
	}else if (c ==LF) { /* Avanzar a la siguiente linea */
		current_column = 0;
		current_line++;
	}else if(c == CR) {
		current_column = 0;
	}

	if (is_printable) { /* Caracter imprimible?*/
		/* Verificar que no se haya llegado al final de la pantalla */
		if (current_column == SCREEN_COLUMNS) {
			current_column = 0;
			current_line ++;
			if (current_line == SCREEN_LINES) {
				console_scroll();
			}
		}
	    /* Escribir el caracter en la memoria de video*/
        videoptr = (unsigned short *)VIDEO_START_ADDR + 
            ((current_line * SCREEN_COLUMNS) + current_column);
		current_column++;
		*videoptr = (text_attributes << 8 | c);
	 }
	console_update_cursor();
}

/**
 * @brief Función para imprimir una cadena de caracteres.
 *
 * Esta rutina valida caracteres especiales, como fin de línea, tabulador y
 * backspace.
 * @param s Cadena terminada en nulo que se desea imprimir
 */
void console_puts(char * s ) {

	char * aux;
	char c;
	int is_printable;
	unsigned int offset;

	aux = s;

	if (aux == 0) {return;}

	while ((c = *aux++) != '\0'){
        console_putchar(c);
	}
}

/**
 * @brief Función para limpiar la pantalla
*/
void console_clear(void) {
	char linea[SCREEN_COLUMNS];
	int i;
	/**
	 * Limpiar la pantalla. Se imprimen SCREEN_LINES lineas, cada linea contiene
	 * SCREEN_COLUMNS espacios.
	 * */
	for (i = 0; i < SCREEN_COLUMNS; i++) {
		linea[i] = SPACE;
	}

	/* Apuntar al inicio de la memoria de video */
	videoptr = (unsigned short *) VIDEO_START_ADDR;
	for (i = 0; i < SCREEN_LINES; i++) {
		console_puts(linea);
	}
	/* Restablecer el apuntador al inicio de la memoria de video*/
	videoptr = (unsigned short *) VIDEO_START_ADDR;
	current_line = 0;
	current_column = 0;
	console_update_cursor();
}

/* Rutinas privadas de stdio.c */

/**
 * @brief Función privada que permite actualizar el cursor en la pantalla.
 */
void console_update_cursor(void) {

	int line;
	int column;

	unsigned int tmp;
	/** Posiciona el cursor de hardware en la pantalla, con la
	 * posicion actual de escritura en la memoria de video.
	 * Para ello, utiliza el microcontrolador CRT, el cual posee dos registros:
	 * - 0x3D4 = registro de indice
	 * - 0x3D5 = registro de datos.
	 *
	 * Para escribir en el microcontrolador CRT, se debe escribir dos veces:
	 * Primero se debe escribir en el registro de índice, para indicar el tipo
	 * de datos que se desea escribir, y luego se escribe en el registro de
	 * datos el dato que se desea escribir.
	 */

	//Scroll: La segunda linea se convierte en la primera, etc.
	if (current_line >= SCREEN_LINES) {
		console_scroll();
	}

	line = current_line;
	column = current_column;

	/* El registro CRT recibe el desplazamiento en bytes desde el inicio
	 * de la memoria de video */
	tmp = (line * SCREEN_COLUMNS) + column;

	/*
	 * 0x3D4 = Registro de indice del CRT. 0x0F = Cursor Location Low: 8 bits
	 * menos significativos de la posicion del cursor
	 * */
	outb(0x3D4, 0x0F);
	/* Escribir los 8 bits menos significativos al puerto */
	outb(0x3D5, tmp);

	/*
	 * 0x3D4 = Registro de indice del CRT. 0x0E = Cursor Location High: 8 bits
	 * mas significativos de la posicion del cursor
	 * */
	outb(0x3D4, 0x0E);
	/* Escribir los 8 bits mas significativos al puerto */
	outb(0x3D5, tmp>> 8);
}

/**
 * @brief Función privada para subir una línea si se ha llegado al final
 * de la pantalla
 */
void console_scroll(void) {
	int i;
	unsigned short * tmp_video;
	/*Apuntar a la segunda linea */
	tmp_video = (unsigned short *)VIDEO_START_ADDR + (SCREEN_COLUMNS);
	//Copiar SCREEN_LINES - 1 lineas de SCREEN_COLUMNS caracteres
	for (i=0; i<(SCREEN_LINES - 1) * SCREEN_COLUMNS; i++) {
			*(tmp_video-SCREEN_COLUMNS) = *tmp_video;
			tmp_video++;
	}

	/* Y luego borrar la ultima linea */
	tmp_video = (unsigned short *)VIDEO_START_ADDR + ((SCREEN_LINES - 1) *
					SCREEN_COLUMNS);
	for (i=0; i< SCREEN_COLUMNS; i++) {
			*(tmp_video) = (text_attributes << 8 | SPACE);
			tmp_video++;
	}
	videoptr = (unsigned short *)VIDEO_START_ADDR + ((SCREEN_LINES - 1) *
						SCREEN_COLUMNS);
	current_line = SCREEN_LINES - 1;
	current_column = 0;
}


/**
 * @brief  Esa funcion implementa en forma basica el comportamiento de
 * 'printf' en C.
 * @param format Formato de la cadena de salida
 * @param ...  Lista de referencias a memoria de las variables a imprimir
 *
*/
void console_printf(char * format,...) {
        char ** arg;
        char c;
        char buf[255];
        char *p;
        int i;

        //Posicionar arg en la dirección de format
        arg = (char **)&format;

        /* Avanzar arg para que apunte al siguiente parametro */
        arg++;

        while ((c = *format++) != '\0') {
                //Buscar el indicador de formato '%'
                if (c != '%') { //Imprimir el caracter
                        console_putchar(c);
                        continue; //Pasar a la siguiente iteracion
                }
                //c = '%', el siguiente caracter indica el tipo de datos
                c = *format++;
                if (c == 'd') { //Entero con signo
                    itoa (*((int *) arg++), buf, 10);
                    console_puts(buf);
                }else if (c == 'u') { //Entero sin signo
                    utoa (*((int *) arg++), buf, 10);
                    console_puts(buf);
                }else if(c == 'x') { //hex
                        itoa (*((int *) arg++), buf, 16);
                        console_puts(buf);
                }else if(c == 'b') { //binario
                        itoa (*((int *) arg++), buf, 2);
                        console_puts(buf);
                }else if(c == 'o') { //octal
                    itoa (*((int *) arg++), buf, 8);
                    console_puts(buf);
                } else if(c == 's') { //String
                        p = *arg++;
                        if (p == 0 || *p == '\0') {
                                console_puts("(null)");
                        }else {
                                console_puts(p);
                        }
                }else { //En caso contrario, mostrar la referencia
                        console_putchar( *((int *) arg++));
                }
        }
}

/**
 * @brief Imprime una cadena en una posicion x, y
 * @param s Cadena terminada en nulo a imprimir
 * @param x Fila
 * @param y Columna
 */
void console_putxy(char * s, short x, short y) {

    int len;
    unsigned int offset;
    unsigned short * ptr;

    offset = (y * SCREEN_COLUMNS) + x;

    len = strlen(s);

    if (offset + len > SCREEN_COLUMNS * SCREEN_LINES) {
        return;
    }

	ptr = (unsigned short *) VIDEO_START_ADDR;

    ptr += offset;

    while (*s != 0) {
        if (*s >= 0x20 && *s < 0x7F) {
            *(ptr++) = (text_attributes << 8 | *s);
        }
        s++;
    }
}

