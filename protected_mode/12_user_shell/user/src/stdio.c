/**
 * @file
 * @brief Salida formateada minima para la biblioteca de usuario.
 */

#include <stdarg.h>
#include <userlib.h>

#define USER_PRINTF_BUFFER_SIZE 256

static char * user_utoa(unsigned int value, char * buffer, unsigned int base);
static char * user_itoa(int value, char * buffer, unsigned int base);

unsigned int user_printf(const char * format, ...) {
    va_list args;
    char buffer[USER_PRINTF_BUFFER_SIZE];
    char number_buffer[32];
    const char * text;
    char * dst;
    char c;

    user_memset(buffer, 0, sizeof(buffer));
    dst = buffer;

    va_start(args, format);

    while ((c = *format++) != 0
            && (unsigned int)(dst - buffer) < (USER_PRINTF_BUFFER_SIZE - 1)) {
        if (c != '%') {
            *dst++ = c;
            continue;
        }

        c = *format++;
        if (c == 0) {
            break;
        }

        if (c == 's') {
            text = va_arg(args, const char *);
            if (text == 0) {
                text = "(null)";
            }

            while (*text != 0
                    && (unsigned int)(dst - buffer) < (USER_PRINTF_BUFFER_SIZE - 1)) {
                *dst++ = *text++;
            }
        } else if (c == 'c') {
            *dst++ = (char)va_arg(args, int);
        } else if (c == 'u') {
            text = user_utoa(va_arg(args, unsigned int), number_buffer, 10);
            while (*text != 0
                    && (unsigned int)(dst - buffer) < (USER_PRINTF_BUFFER_SIZE - 1)) {
                *dst++ = *text++;
            }
        } else if (c == 'd') {
            text = user_itoa(va_arg(args, int), number_buffer, 10);
            while (*text != 0
                    && (unsigned int)(dst - buffer) < (USER_PRINTF_BUFFER_SIZE - 1)) {
                *dst++ = *text++;
            }
        } else if (c == 'x') {
            text = user_utoa(va_arg(args, unsigned int), number_buffer, 16);
            while (*text != 0
                    && (unsigned int)(dst - buffer) < (USER_PRINTF_BUFFER_SIZE - 1)) {
                *dst++ = *text++;
            }
        } else if (c == '%') {
            *dst++ = '%';
        } else {
            *dst++ = '%';
            if ((unsigned int)(dst - buffer) < (USER_PRINTF_BUFFER_SIZE - 1)) {
                *dst++ = c;
            }
        }
    }

    *dst = 0;
    va_end(args);

    return sys_write(buffer, (unsigned int)(dst - buffer));
}

static char * user_utoa(unsigned int value, char * buffer, unsigned int base) {
    char scratch[32];
    char * start;
    unsigned int i;
    unsigned int remainder;

    if (base < 2 || base > 16) {
        buffer[0] = 0;
        return buffer;
    }

    start = buffer;

    i = 0;
    do {
        remainder = value % base;
        if (remainder < 10) {
            scratch[i++] = (char)('0' + remainder);
        } else {
            scratch[i++] = (char)('a' + (remainder - 10));
        }
        value /= base;
    } while (value > 0);

    while (i > 0) {
        *buffer++ = scratch[--i];
    }
    *buffer = 0;

    return start;
}

static char * user_itoa(int value, char * buffer, unsigned int base) {
    unsigned int magnitude;
    char * result;

    if (base != 10) {
        return user_utoa((unsigned int)value, buffer, base);
    }

    if (value < 0) {
        *buffer++ = '-';
        magnitude = (unsigned int)(-value);
        result = user_utoa(magnitude, buffer, base);
        return buffer - 1;
    }

    return user_utoa((unsigned int)value, buffer, base);
}
