/**
 * @file
 * @brief Wrappers minimos de llamadas al sistema para programas de usuario.
 */

#include <userlib.h>

static unsigned int user_syscall0(unsigned int number);
static unsigned int user_syscall1(unsigned int number, unsigned int arg0);
static unsigned int user_syscall2(unsigned int number, unsigned int arg0,
        unsigned int arg1);
static unsigned int user_syscall3(unsigned int number, unsigned int arg0,
        unsigned int arg1, unsigned int arg2);

unsigned int sys_write(const char * text, unsigned int len) {
    return user_syscall2(USERLIB_SYSCALL_WRITE, (unsigned int)text, len);
}

void sys_exit(unsigned int code) {
    user_syscall2(USERLIB_SYSCALL_EXIT, code, 0);

    for (;;) {
        __asm__ volatile("jmp .");
    }
}

unsigned int sys_get_ticks(void) {
    return user_syscall0(USERLIB_SYSCALL_GET_TICKS);
}

unsigned int sys_read_char(void) {
    return user_syscall0(USERLIB_SYSCALL_READ_CHAR);
}

int sys_exec(const char * path) {
    return (int)user_syscall1(USERLIB_SYSCALL_EXEC, (unsigned int)path);
}

int sys_list_dir(const char * path, char * buffer, unsigned int buffer_size) {
    return (int)user_syscall3(USERLIB_SYSCALL_LIST_DIR, (unsigned int)path,
            (unsigned int)buffer, buffer_size);
}

int sys_read_file(const char * path, char * buffer, unsigned int buffer_size) {
    return (int)user_syscall3(USERLIB_SYSCALL_READ_FILE, (unsigned int)path,
            (unsigned int)buffer, buffer_size);
}

static unsigned int user_syscall0(unsigned int number) {
    unsigned int result;

    __asm__ volatile("int %1"
            : "=a"(result)
            : "i"(USERLIB_SYSCALL_INTERRUPT), "a"(number)
            : "memory");

    return result;
}

static unsigned int user_syscall1(unsigned int number, unsigned int arg0) {
    unsigned int result;

    __asm__ volatile("int %1"
            : "=a"(result)
            : "i"(USERLIB_SYSCALL_INTERRUPT), "a"(number), "b"(arg0)
            : "memory");

    return result;
}

static unsigned int user_syscall2(unsigned int number, unsigned int arg0,
        unsigned int arg1) {
    unsigned int result;

    __asm__ volatile("int %1"
            : "=a"(result)
            : "i"(USERLIB_SYSCALL_INTERRUPT), "a"(number), "b"(arg0), "c"(arg1)
            : "memory");

    return result;
}

static unsigned int user_syscall3(unsigned int number, unsigned int arg0,
        unsigned int arg1, unsigned int arg2) {
    unsigned int result;

    __asm__ volatile("int %1"
            : "=a"(result)
            : "i"(USERLIB_SYSCALL_INTERRUPT), "a"(number), "b"(arg0),
              "c"(arg1), "d"(arg2)
            : "memory");

    return result;
}
