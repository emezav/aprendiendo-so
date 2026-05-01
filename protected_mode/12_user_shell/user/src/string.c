/**
 * @file
 * @brief Utilidades minimas de cadena para la biblioteca de usuario.
 */

#include <userlib.h>

void * user_memcpy(void * dst, const void * src, unsigned int count) {
    const char * src_bytes;
    char * dst_bytes;

    src_bytes = (const char *)src;
    dst_bytes = (char *)dst;

    while (count > 0) {
        *dst_bytes++ = *src_bytes++;
        count--;
    }

    return dst;
}

void * user_memset(void * dst, char value, unsigned int count) {
    char * dst_bytes;

    dst_bytes = (char *)dst;
    while (count > 0) {
        *dst_bytes++ = value;
        count--;
    }

    return dst;
}

unsigned int user_strlen(const char * text) {
    unsigned int len;

    if (text == 0) {
        return 0;
    }

    len = 0;
    while (text[len] != 0) {
        len++;
    }

    return len;
}

int user_strcmp(const char * left, const char * right) {
    while (*left != 0 && *right != 0 && *left == *right) {
        left++;
        right++;
    }

    if (*left == 0 && *right == 0) {
        return 0;
    }

    if (*left == 0) {
        return -1;
    }

    if (*right == 0) {
        return 1;
    }

    return ((int)*right) - ((int)*left);
}

int user_strncmp(const char * left, const char * right, unsigned int count) {
    while (count > 0 && *left != 0 && *right != 0 && *left == *right) {
        left++;
        right++;
        count--;
    }

    if (count == 0) {
        return 0;
    }

    if (*left == 0 && *right == 0) {
        return 0;
    }

    return ((int)*right) - ((int)*left);
}

unsigned int user_puts(const char * text) {
    return sys_write(text, user_strlen(text));
}
