/**
 * @file
 * @brief Utilidad minima para mostrar el contenido de `/boot/grub/menu.lst`.
 */

#include <stdlib.h>
#include <userlib.h>

#define CAT_BUFFER_SIZE 1024

int main(void) {
    static char content[CAT_BUFFER_SIZE + 1];
    int size;

    size = sys_read_file("/boot/grub/menu.lst", content, CAT_BUFFER_SIZE);
    if (size < 0) {
        user_printf("cat: could not read /boot/grub/menu.lst\n");
        return 1;
    }

    content[size] = 0;
    user_printf("%s\n", content);
    return 0;
}
