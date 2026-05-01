/**
 * @file
 * @brief Utilidad minima para listar el contenido de `/boot`.
 */

#include <stdlib.h>
#include <userlib.h>

#define LS_BUFFER_SIZE 1024

int main(void) {
    static char listing[LS_BUFFER_SIZE + 1];
    int size;

    size = sys_list_dir("/boot", listing, LS_BUFFER_SIZE);
    if (size < 0) {
        user_printf("ls: could not list /boot\n");
        return 1;
    }

    listing[size] = 0;
    user_printf("ls: /boot\n%s", listing);
    return 0;
}
