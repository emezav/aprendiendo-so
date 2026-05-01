/**
 * @file
 * @brief Primer programa de usuario escrito en C sobre la libc minima.
 */

#include <userlib.h>

int main(void) {
    unsigned int ticks;

    user_printf("hello from libc via %s\n", "/boot/hello.elf");
    ticks = sys_get_ticks();
    user_printf("ticks=%u hex=%x\n", ticks, ticks);

    return ticks & 0xff;
}
