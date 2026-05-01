/**
 * @file
 * @ingroup kernel_code
 * @brief Dispatcher minimo de syscalls y prueba inicial de `int 0x80`.
 */

#include <asm.h>
#include <klog.h>
#include <monitor.h>
#include <paging.h>
#include <physmem.h>
#include <pm.h>
#include <serial.h>
#include <string.h>
#include <syscall.h>
#include <timer.h>
#include <tss.h>
#include <userexec.h>
#include <kernel.h>
#include <ext2.h>

/** @brief Pagina virtual del payload de prueba de syscalls. */
#define SYSCALL_TEST_CODE_VADDR (USER_VIRT_START + (PAGE_SIZE * 2))
/** @brief Pagina virtual de la pila del payload de prueba de syscalls. */
#define SYSCALL_TEST_STACK_VADDR (USER_VIRT_START + (PAGE_SIZE * 3))
/** @brief Tope inicial de la pila del payload de prueba de syscalls. */
#define SYSCALL_TEST_STACK_TOP (SYSCALL_TEST_STACK_VADDR + PAGE_SIZE)
/** @brief Desplazamiento del mensaje dentro de la pagina de codigo. */
#define SYSCALL_TEST_MESSAGE_OFFSET 64
/** @brief Maximo de bytes aceptados por la syscall `write` en este corte. */
#define SYSCALL_WRITE_MAX_LEN 255
/** @brief Valor de retorno usado para syscalls no soportadas o invalidas. */
#define SYSCALL_ERROR ((unsigned int)-1)
/** @brief Longitud maxima aceptada para una ruta pasada a `exec`. */
#define SYSCALL_EXEC_PATH_MAX 127
/** @brief Maximo de bytes copiados a un buffer de listado de directorio. */
#define SYSCALL_LIST_MAX EXT2_LIST_MAX
/** @brief Maximo de bytes copiados a un buffer de lectura de archivo. */
#define SYSCALL_READ_FILE_MAX EXT2_FILE_READ_MAX

/** @brief Mensaje minimo escrito por el payload de prueba via `int 0x80`. */
static const char syscall_test_message[] = "syscall: hello from ring 3 via int 0x80\n";

/** @brief Payload minimo que invoca `write` y luego `exit`. */
static unsigned char syscall_test_code[] = {
    0xB8, 0, 0, 0, 0, /* mov eax, SYSCALL_WRITE */
    0xBB, 0, 0, 0, 0, /* mov ebx, message_ptr */
    0xB9, 0, 0, 0, 0, /* mov ecx, message_len */
    0xCD, SYSCALL_INTERRUPT, /* int 0x80 */
    0xB8, 0, 0, 0, 0, /* mov eax, SYSCALL_EXIT */
    0xBB, 0, 0, 0, 0, /* mov ebx, exit_code */
    0xCD, SYSCALL_INTERRUPT, /* int 0x80 */
    0xEB, 0xFE /* jmp $ */
};

/** @brief Payload minimo que obtiene ticks y luego usa `exit` para reportarlos. */
static unsigned char syscall_ticks_test_code[] = {
    0xB8, 0, 0, 0, 0, /* mov eax, SYSCALL_GET_TICKS */
    0xCD, SYSCALL_INTERRUPT, /* int 0x80 */
    0x89, 0xC3, /* mov ebx, eax */
    0xB8, 0, 0, 0, 0, /* mov eax, SYSCALL_EXIT */
    0xCD, SYSCALL_INTERRUPT, /* int 0x80 */
    0xEB, 0xFE /* jmp $ */
};

/** @brief Indica si las paginas de la prueba ya fueron preparadas. */
static int syscall_test_prepared;

/** @brief Cuenta cuantas syscalls se atendieron desde el payload de prueba. */
static unsigned int syscall_count;

/** @brief Indica si la prueba de `int 0x80` fue lanzada desde el monitor. */
static int syscall_test_active;

extern unsigned int kernel_stack_end;

/**
 * @brief Prepara el payload de prueba de syscalls y su pila.
 * @return 1 si la preparacion fue exitosa, 0 en caso contrario.
 */
static int prepare_syscall_test(void);

/**
 * @brief Copia el payload y su mensaje a la pagina de codigo de userland.
 */
static void write_syscall_test_code(void);

/**
 * @brief Ejecuta el payload de prueba de syscalls en `ring 3`.
 */
static void enter_syscall_test(void);

/**
 * @brief Callback del comando `syscall`.
 * @param args Argumentos crudos del comando. No se usan en esta version.
 */
static void syscall_command_enter(char * args);

/**
 * @brief Callback del comando `systicks`.
 * @param args Argumentos crudos del comando. No se usan en esta version.
 */
static void syscall_command_ticks(char * args);

/**
 * @brief Atiende la interrupcion `0x80` y despacha la syscall pedida.
 * @param state Marco de interrupcion capturado por el kernel.
 */
static void syscall_interrupt_handler(interrupt_state * state);

/**
 * @brief Implementa la syscall minima `write`.
 * @param state Marco de interrupcion con argumentos y valor de retorno.
 */
static void syscall_handle_write(interrupt_state * state);

/**
 * @brief Implementa la syscall minima `exit`.
 * @param state Marco de interrupcion con argumentos y valor de retorno.
 */
static void syscall_handle_exit(interrupt_state * state);

/**
 * @brief Implementa la syscall minima `get_ticks`.
 * @param state Marco de interrupcion con argumentos y valor de retorno.
 */
static void syscall_handle_get_ticks(interrupt_state * state);
static void syscall_handle_read_char(interrupt_state * state);
static void syscall_handle_exec(interrupt_state * state);
static void syscall_handle_list_dir(interrupt_state * state);
static void syscall_handle_read_file(interrupt_state * state);
static int syscall_copy_user_string(unsigned int user_ptr, char * dst,
        unsigned int dst_size);

/**
 * @brief Inyecta el payload indicado en la pagina de codigo de userland.
 * @param code Buffer con las instrucciones a copiar.
 * @param code_size Cantidad de bytes del payload.
 * @param include_message 1 si tambien se debe copiar el mensaje base, 0 si no.
 */
static void load_syscall_test_code(const unsigned char * code, unsigned int code_size,
        int include_message);

void setup_syscalls(void) {
    setup_idt_descriptor(SYSCALL_INTERRUPT, KERNEL_CODE_SELECTOR,
            isr_table[SYSCALL_INTERRUPT], RING3_DPL, INTERRUPT_GATE_TYPE);
    install_interrupt_handler(SYSCALL_INTERRUPT, syscall_interrupt_handler);
}

void register_syscall_monitor_commands(void) {
    monitor_register_command("syscall", syscall_command_enter,
            "enter ring 3, invoke int 0x80 write and then exit");
    monitor_register_command("systicks", syscall_command_ticks,
            "enter ring 3, read timer ticks via int 0x80 and then exit");
}

static int prepare_syscall_test(void) {
    unsigned int code_frame;
    unsigned int stack_frame;

    if (syscall_test_prepared) {
        return 1;
    }

    code_frame = allocate_frame();
    if (!code_frame) {
        return 0;
    }

    stack_frame = allocate_frame();
    if (!stack_frame) {
        free_frame(code_frame);
        return 0;
    }

    if (!map_user_page(SYSCALL_TEST_CODE_VADDR, code_frame)) {
        free_frame(code_frame);
        free_frame(stack_frame);
        return 0;
    }

    if (!map_user_page(SYSCALL_TEST_STACK_VADDR, stack_frame)) {
        destroy_page(SYSCALL_TEST_CODE_VADDR);
        free_frame(stack_frame);
        return 0;
    }

    write_syscall_test_code();
    syscall_test_prepared = 1;
    return 1;
}

static void write_syscall_test_code(void) {
    load_syscall_test_code(syscall_test_code, sizeof(syscall_test_code), 1);
}

static void enter_syscall_test(void) {
    unsigned int user_code;
    unsigned int user_data;

    user_code = user_code_selector | 0x03;
    user_data = user_data_selector | 0x03;

    serial_printf("[syscall] entering ring3 code=0x%x stack_top=0x%x int=0x%x\n",
            SYSCALL_TEST_CODE_VADDR, SYSCALL_TEST_STACK_TOP,
            SYSCALL_INTERRUPT);

    inline_assembly(".intel_syntax noprefix\n\t"
                    "mov ds, %w0\n\t"
                    "mov es, %w0\n\t"
                    "mov fs, %w0\n\t"
                    "mov gs, %w0\n\t"
                    "cli\n\t"
                    "push %0\n\t"
                    "push %1\n\t"
                    "push %2\n\t"
                    "push %3\n\t"
                    "push %4\n\t"
                    "iret\n\t"
                    ".att_syntax prefix\n\t"
                    :
                    : "r"(user_data),
                      "r"(SYSCALL_TEST_STACK_TOP),
                      "r"(IF_ENABLE),
                      "r"(user_code),
                      "r"(SYSCALL_TEST_CODE_VADDR)
                    : "memory");
}

static void syscall_command_enter(char * args) {
    (void)args;

    if (user_code_selector == 0 || user_data_selector == 0) {
        klog_printf("syscall: user segments are not initialized\n");
        return;
    }

    if (kernel_tss_selector == 0) {
        klog_printf("syscall: tss is not initialized\n");
        serial_printf("[syscall] missing tss\n");
        return;
    }

    if (!prepare_syscall_test()) {
        klog_printf("syscall: could not prepare user pages\n");
        serial_printf("[syscall] prepare failed\n");
        return;
    }

    klog_printf("syscall: launching first int 0x80 test\n");
    syscall_test_active = 1;
    write_syscall_test_code();
    klog_printf("syscall: expected flow => write runs, then exit halts the kernel\n");
    enter_syscall_test();

    klog_printf("syscall: unexpected return to monitor command path\n");
}

static void syscall_command_ticks(char * args) {
    (void)args;

    if (user_code_selector == 0 || user_data_selector == 0) {
        klog_printf("systicks: user segments are not initialized\n");
        return;
    }

    if (kernel_tss_selector == 0) {
        klog_printf("systicks: tss is not initialized\n");
        serial_printf("[systicks] missing tss\n");
        return;
    }

    if (!prepare_syscall_test()) {
        klog_printf("systicks: could not prepare user pages\n");
        serial_printf("[systicks] prepare failed\n");
        return;
    }

    syscall_test_active = 1;
    load_syscall_test_code(syscall_ticks_test_code,
            sizeof(syscall_ticks_test_code), 0);
    klog_printf("systicks: launching get_ticks test via int 0x80\n");
    klog_printf("systicks: expected flow => get_ticks returns in eax, then exit reports it\n");
    enter_syscall_test();

    klog_printf("systicks: unexpected return to monitor command path\n");
}

static void syscall_interrupt_handler(interrupt_state * state) {
    syscall_count++;
    if (state->eax != SYSCALL_READ_CHAR) {
        serial_printf("[syscall] dispatch number=%u eax=%u ebx=0x%x ecx=%u count=%u\n",
                state->number, state->eax, state->ebx, state->ecx, syscall_count);
    }

    switch (state->eax) {
    case SYSCALL_WRITE:
        syscall_handle_write(state);
        return;
    case SYSCALL_EXIT:
        syscall_handle_exit(state);
        return;
    case SYSCALL_GET_TICKS:
        syscall_handle_get_ticks(state);
        return;
    case SYSCALL_READ_CHAR:
        syscall_handle_read_char(state);
        return;
    case SYSCALL_EXEC:
        syscall_handle_exec(state);
        return;
    case SYSCALL_LIST_DIR:
        syscall_handle_list_dir(state);
        return;
    case SYSCALL_READ_FILE:
        syscall_handle_read_file(state);
        return;
    default:
        klog_printf("syscall: unsupported number=%u\n", state->eax);
        serial_printf("[syscall] unsupported number=%u\n", state->eax);
        state->eax = SYSCALL_ERROR;
        return;
    }
}

static void syscall_handle_write(interrupt_state * state) {
    const char * user_text;
    unsigned int i;
    unsigned int len;
    unsigned int max_len;
    char buffer[SYSCALL_WRITE_MAX_LEN + 1];

    user_text = (const char *)state->ebx;
    len = state->ecx;

    if (user_text == 0 || len == 0 || !is_user_vaddr((unsigned int)user_text)) {
        klog_printf("syscall: invalid write buffer\n");
        serial_printf("[syscall] invalid write buffer ptr=0x%x len=%u\n",
                state->ebx, len);
        state->eax = SYSCALL_ERROR;
        return;
    }

    max_len = USER_VIRT_END - (unsigned int)user_text;
    if (len > max_len || len > SYSCALL_WRITE_MAX_LEN) {
        klog_printf("syscall: write length out of range\n");
        serial_printf("[syscall] invalid write length ptr=0x%x len=%u max=%u\n",
                state->ebx, len, max_len);
        state->eax = SYSCALL_ERROR;
        return;
    }

    for (i = 0; i < len; i++) {
        buffer[i] = user_text[i];
    }
    buffer[len] = 0;

    klog_printf("%s", buffer);
    serial_printf("[syscall] write completed len=%u\n", len);
    state->eax = len;
}

static void syscall_handle_exit(interrupt_state * state) {
    klog_printf("syscall: exit(%u) requested from ring 3\n", state->ebx);
    serial_printf("[syscall] exit code=%u old_cs=0x%x old_eip=0x%x\n",
            state->ebx, state->old_cs, state->old_eip);
    state->eax = 0;
    syscall_test_active = 0;

    klog_printf("syscall: user program finished; returning to kernel monitor\n");
    serial_printf("[syscall] switching to kernel stack and returning to monitor\n");
    inline_assembly(".intel_syntax noprefix\n\t"
                    "mov esp, OFFSET kernel_stack_end\n\t"
                    "jmp kernel_resume_monitor_after_user_exit\n\t"
                    ".att_syntax prefix\n\t");
}

static void syscall_handle_get_ticks(interrupt_state * state) {
    state->eax = timer_get_ticks();
    serial_printf("[syscall] get_ticks -> %u\n", state->eax);
}

static void syscall_handle_read_char(interrupt_state * state) {
    if (keyboard_has_char()) {
        state->eax = (unsigned int)(unsigned char)keyboard_getchar();
        return;
    }

    state->eax = 0;
}

static void syscall_handle_exec(interrupt_state * state) {
    char path_buffer[SYSCALL_EXEC_PATH_MAX + 1];

    if (!syscall_copy_user_string(state->ebx, path_buffer, sizeof(path_buffer))) {
        serial_printf("[syscall] invalid exec path ptr=0x%x\n", state->ebx);
        state->eax = SYSCALL_ERROR;
        return;
    }
    serial_printf("[syscall] exec path=%s\n", path_buffer);
    if (!userexec_run_path(path_buffer)) {
        state->eax = SYSCALL_ERROR;
        return;
    }

    state->eax = 0;
}

static void syscall_handle_list_dir(interrupt_state * state) {
    char path_buffer[SYSCALL_EXEC_PATH_MAX + 1];
    char * user_buffer;
    unsigned int max_len;
    unsigned int size;

    if (!syscall_copy_user_string(state->ebx, path_buffer, sizeof(path_buffer))) {
        serial_printf("[syscall] invalid list_dir path ptr=0x%x\n", state->ebx);
        state->eax = SYSCALL_ERROR;
        return;
    }

    user_buffer = (char *)state->ecx;
    max_len = state->edx;
    if (user_buffer == 0 || max_len == 0 || !is_user_vaddr((unsigned int)user_buffer)) {
        serial_printf("[syscall] invalid list_dir buffer ptr=0x%x len=%u\n",
                state->ecx, max_len);
        state->eax = SYSCALL_ERROR;
        return;
    }

    if (max_len > SYSCALL_LIST_MAX) {
        max_len = SYSCALL_LIST_MAX;
    }
    if (max_len > (USER_VIRT_END - (unsigned int)user_buffer)) {
        state->eax = SYSCALL_ERROR;
        return;
    }

    memset(user_buffer, 0, max_len);
    if (!ext2_list_dir(path_buffer, user_buffer, max_len, &size)) {
        serial_printf("[syscall] list_dir failed path=%s\n", path_buffer);
        state->eax = SYSCALL_ERROR;
        return;
    }

    serial_printf("[syscall] list_dir path=%s size=%u\n", path_buffer, size);
    state->eax = size;
}

static void syscall_handle_read_file(interrupt_state * state) {
    char path_buffer[SYSCALL_EXEC_PATH_MAX + 1];
    char * user_buffer;
    unsigned int max_len;
    unsigned int size;

    if (!syscall_copy_user_string(state->ebx, path_buffer, sizeof(path_buffer))) {
        serial_printf("[syscall] invalid read_file path ptr=0x%x\n", state->ebx);
        state->eax = SYSCALL_ERROR;
        return;
    }

    user_buffer = (char *)state->ecx;
    max_len = state->edx;
    if (user_buffer == 0 || max_len == 0 || !is_user_vaddr((unsigned int)user_buffer)) {
        serial_printf("[syscall] invalid read_file buffer ptr=0x%x len=%u\n",
                state->ecx, max_len);
        state->eax = SYSCALL_ERROR;
        return;
    }

    if (max_len > SYSCALL_READ_FILE_MAX) {
        max_len = SYSCALL_READ_FILE_MAX;
    }
    if (max_len > (USER_VIRT_END - (unsigned int)user_buffer)) {
        state->eax = SYSCALL_ERROR;
        return;
    }

    memset(user_buffer, 0, max_len);
    if (!ext2_read_file(path_buffer, user_buffer, max_len, &size)) {
        serial_printf("[syscall] read_file failed path=%s\n", path_buffer);
        state->eax = SYSCALL_ERROR;
        return;
    }

    serial_printf("[syscall] read_file path=%s size=%u\n", path_buffer, size);
    state->eax = size;
}

static int syscall_copy_user_string(unsigned int user_ptr, char * dst,
        unsigned int dst_size) {
    const char * user_text;
    unsigned int max_len;
    unsigned int i;

    if (dst == 0 || dst_size == 0) {
        return 0;
    }

    user_text = (const char *)user_ptr;
    if (user_text == 0 || !is_user_vaddr((unsigned int)user_text)) {
        return 0;
    }

    max_len = USER_VIRT_END - (unsigned int)user_text;
    if (max_len > (dst_size - 1)) {
        max_len = dst_size - 1;
    }

    for (i = 0; i < max_len; i++) {
        dst[i] = user_text[i];
        if (dst[i] == 0) {
            return 1;
        }
    }

    dst[max_len] = 0;
    return 0;
}

static void load_syscall_test_code(const unsigned char * code, unsigned int code_size,
        int include_message) {
    unsigned char * code_ptr;
    char * message_ptr;
    unsigned int message_vaddr;
    unsigned int message_len;

    code_ptr = (unsigned char *)SYSCALL_TEST_CODE_VADDR;
    memcpy(code_ptr, code, code_size);

    if (include_message) {
        message_ptr = (char *)(SYSCALL_TEST_CODE_VADDR + SYSCALL_TEST_MESSAGE_OFFSET);
        message_vaddr = (unsigned int)message_ptr;
        message_len = (unsigned int)(sizeof(syscall_test_message) - 1);

        memcpy(message_ptr, syscall_test_message, sizeof(syscall_test_message));
        *((unsigned int *)(code_ptr + 1)) = SYSCALL_WRITE;
        *((unsigned int *)(code_ptr + 6)) = message_vaddr;
        *((unsigned int *)(code_ptr + 11)) = message_len;
        *((unsigned int *)(code_ptr + 18)) = SYSCALL_EXIT;
        *((unsigned int *)(code_ptr + 23)) = 0;
        return;
    }

    *((unsigned int *)(code_ptr + 1)) = SYSCALL_GET_TICKS;
    *((unsigned int *)(code_ptr + 10)) = SYSCALL_EXIT;
}
