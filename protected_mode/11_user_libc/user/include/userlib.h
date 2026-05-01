/**
 * @file
 * @brief Biblioteca minima de usuario para `11_user_libc`.
 */

#ifndef USERLIB_H_
#define USERLIB_H_

/** @brief Vector reservado para las llamadas al sistema de usuario. */
#define USERLIB_SYSCALL_INTERRUPT 0x80
/** @brief Numero de la syscall `write`. */
#define USERLIB_SYSCALL_WRITE 1
/** @brief Numero de la syscall `exit`. */
#define USERLIB_SYSCALL_EXIT 2
/** @brief Numero de la syscall `get_ticks`. */
#define USERLIB_SYSCALL_GET_TICKS 3

unsigned int sys_write(const char * text, unsigned int len);
void sys_exit(unsigned int code);
unsigned int sys_get_ticks(void);

void * user_memcpy(void * dst, const void * src, unsigned int count);
void * user_memset(void * dst, char value, unsigned int count);
unsigned int user_strlen(const char * text);
int user_strcmp(const char * left, const char * right);
int user_strncmp(const char * left, const char * right, unsigned int count);
unsigned int user_puts(const char * text);
unsigned int user_printf(const char * format, ...);

#endif /* USERLIB_H_ */
