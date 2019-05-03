/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Contiene las primitivas basicas para entrada / salida por
 * el puerto serial COM1
 */

#include <pm.h>
#include <asm.h>
#include <serial.h>
#include <stdlib.h>

/**
 * Inicializa el puerto serial COM1
 */
void init_serial() {
   outb(COM1_PORT + 1, 0x00);    // Disable all interrupts
   outb(COM1_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(COM1_PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(COM1_PORT + 1, 0x00);    //                  (hi byte)
   outb(COM1_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(COM1_PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(COM1_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

/**
 * Verifica si el puerto serial se encuentra vacío
*/
int is_transmit_empty() {
   return inb(COM1_PORT + 5) & 0x20;
}
 
/**
 * Escribe un caracter en el puerto serial
*/
void serial_putchar(char a) {
   while (is_transmit_empty() == 0);
 
   outb(COM1_PORT,a);
}

/**
 * Escribe una cadena de caracteres en el puerto serial
 */
void serial_puts(char * s) {
    char * aux;
    aux = s;
    while (*aux != 0) {
        serial_putchar(*aux);
        aux++;
    }
}

/**
 * @brief  Esa funcion implementa en forma basica el comportamiento de
 * 'printf' en C.
 * @param format Formato de la cadena de salida
 * @param ...  Lista de referencias a memoria de las variables a imprimir
 *
*/
void serial_printf(char * format,...) {
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
                        serial_putchar(c);
                        continue; //Pasar a la siguiente iteracion
                }
                //c = '%', el siguiente caracter indica el tipo de datos
                c = *format++;
                if (c == 'd') { //Entero con signo
                    itoa (*((int *) arg++), buf, 10);
                    serial_puts(buf);
                }else if (c == 'u') { //Entero sin signo
                    utoa (*((int *) arg++), buf, 10);
                    serial_puts(buf);
                }else if(c == 'x') { //hex
                        itoa (*((int *) arg++), buf, 16);
                        serial_puts(buf);
                }else if(c == 'b') { //binario
                        itoa (*((int *) arg++), buf, 2);
                        serial_puts(buf);
                }else if(c == 'o') { //octal
                    itoa (*((int *) arg++), buf, 8);
                    serial_puts(buf);
                } else if(c == 's') { //String
                        p = *arg++;
                        if (p == 0 || *p == '\0') {
                                serial_puts("(null)");
                        }else {
                                serial_puts(p);
                        }
                }else { //En caso contrario, mostrar la referencia
                        serial_putchar( *((int *) arg++));
                }
        }
}
