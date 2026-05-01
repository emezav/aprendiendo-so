/**
 * @file
 * @ingroup kernel_code
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License.
 *
 * @brief Implementacion de salida dual para consola y serial.
 */

#include <console.h>
#include <klog.h>
#include <serial.h>
#include <stdlib.h>
#include <string.h>

void klog_printf(char * format, ...) {
    char buffer[BUFSIZ];
    char ** arg;
    char c;
    char number_buffer[255];
    char * text;
    char * dst;

    memset(buffer, 0, sizeof(buffer));
    dst = buffer;

    arg = (char **)&format;
    arg++;

    while ((c = *format++) != '\0') {
        if (c != '%') {
            *dst++ = c;
            *dst = 0;
            continue;
        }

        c = *format++;
        if (c == 'd') {
            itoa(*((int *) arg++), number_buffer, 10);
            strcat(dst, number_buffer);
            dst += strlen(number_buffer);
        } else if (c == 'u') {
            utoa(*((int *) arg++), number_buffer, 10);
            strcat(dst, number_buffer);
            dst += strlen(number_buffer);
        } else if (c == 'x') {
            itoa(*((int *) arg++), number_buffer, 16);
            strcat(dst, number_buffer);
            dst += strlen(number_buffer);
        } else if (c == 'b') {
            itoa(*((int *) arg++), number_buffer, 2);
            strcat(dst, number_buffer);
            dst += strlen(number_buffer);
        } else if (c == 'o') {
            itoa(*((int *) arg++), number_buffer, 8);
            strcat(dst, number_buffer);
            dst += strlen(number_buffer);
        } else if (c == 's') {
            text = *arg++;
            if (text != 0 && *text != 0) {
                strcat(dst, text);
                dst += strlen(text);
            }
        } else {
            *dst++ = *((int *) arg++);
            *dst = 0;
        }
    }

    console_puts(buffer);
    serial_puts(buffer);
}
