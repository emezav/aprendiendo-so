/**
 * @file
 * @ingroup kernel_code
 * @brief ABI minima de syscalls para la etapa inicial de `07_syscalls`.
 */

#ifndef SYSCALL_H_
#define SYSCALL_H_

/** @brief Vector de interrupcion reservado para la ABI de syscalls. */
#define SYSCALL_INTERRUPT 0x80

/** @brief Syscall para escribir una cadena visible en consola y serial. */
#define SYSCALL_WRITE 1

/** @brief Syscall para terminar la prueba de userland actual. */
#define SYSCALL_EXIT 2

/** @brief Syscall para obtener el contador actual de ticks del sistema. */
#define SYSCALL_GET_TICKS 3
/** @brief Syscall no bloqueante para leer un caracter del teclado. */
#define SYSCALL_READ_CHAR 4
/** @brief Syscall para reemplazar el programa actual por otro ELF. */
#define SYSCALL_EXEC 5
/** @brief Syscall para listar un directorio de `ext2` en un buffer de usuario. */
#define SYSCALL_LIST_DIR 6
/** @brief Syscall para leer un archivo regular de `ext2` en un buffer de usuario. */
#define SYSCALL_READ_FILE 7

/**
 * @brief Configura `int 0x80` como puerta de syscalls accesible desde `ring 3`.
 */
void setup_syscalls(void);

/**
 * @brief Registra en el monitor los comandos de prueba asociados a syscalls.
 */
void register_syscall_monitor_commands(void);

#endif /* SYSCALL_H_ */
