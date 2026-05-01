/**
 * @file
 * @brief Capa minima de stdlib sobre la ABI cruda de syscalls.
 */

#include <stdlib.h>
#include <userlib.h>

void exit(unsigned int code) {
    sys_exit(code);
}
