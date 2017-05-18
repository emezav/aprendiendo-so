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
void write_serial(char a) {
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
        write_serial(*aux);
        aux++;
    }
}