/**
 * @file
 * @brief Utilidad minima de identificacion del sistema.
 */

#include <stdlib.h>
#include <userlib.h>

int main(void) {
    user_printf("Aprendiendo SO - protected_mode/13_user_programs\n");
    user_printf("ticks=%u\n", sys_get_ticks());
    return 0;
}
