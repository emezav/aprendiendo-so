/**
 * @file
 * @ingroup kernel_code
 * @brief Prueba minima para entrar a `ring 3` y volver al kernel por `0x81`.
 */

#include <asm.h>
#include <klog.h>
#include <monitor.h>
#include <paging.h>
#include <physmem.h>
#include <pm.h>
#include <serial.h>
#include <tss.h>
#include <usermode.h>

/** @brief Direccion virtual de la pagina que contiene el payload de userland. */
#define USERMODE_TEST_CODE_VADDR USER_VIRT_START
/** @brief Direccion virtual de la pagina usada como pila de userland. */
#define USERMODE_TEST_STACK_VADDR (USER_VIRT_START + PAGE_SIZE)
/** @brief Tope inicial de la pila de userland. */
#define USERMODE_TEST_STACK_TOP (USERMODE_TEST_STACK_VADDR + PAGE_SIZE)

/**
 * @brief Payload minimo de userland.
 * @details Ejecuta `int 0x81` y luego queda en un bucle local. El flujo normal
 * de esta prueba es que el kernel recupere el control en el manejador de
 * `USERMODE_TEST_INTERRUPT`.
 */
static const unsigned char usermode_test_code[] = {
    0xCD, USERMODE_TEST_INTERRUPT, /* int 0x81 */
    0xEB, 0xFE                     /* jmp $ */
};

/** @brief Indica si las paginas de la prueba ya fueron preparadas. */
static int usermode_test_prepared;

/** @brief Cuenta cuantas veces se atendio la interrupcion de prueba. */
static unsigned int usermode_test_interrupt_count;

/**
 * @brief Prepara el payload minimo de userland y su pila.
 * @return 1 si la preparacion fue exitosa, 0 en caso contrario.
 */
static int prepare_usermode_test(void);

/**
 * @brief Copia el payload minimo a la pagina de codigo de userland.
 */
static void write_usermode_test_code(void);

/**
 * @brief Ejecuta la primera transicion controlada a `ring 3`.
 */
static void enter_usermode_test(void);

/**
 * @brief Callback del comando `usermode`.
 * @param args Argumentos crudos del comando. No se usan en esta version.
 */
static void usermode_command_enter(char * args);

/**
 * @brief Atiende la interrupcion `0x81` generada por el payload de userland.
 * @param state Marco de interrupcion capturado por el kernel.
 */
static void usermode_test_interrupt_handler(interrupt_state * state);

void setup_usermode(void) {
    setup_idt_descriptor(USERMODE_TEST_INTERRUPT, KERNEL_CODE_SELECTOR,
            isr_table[USERMODE_TEST_INTERRUPT], RING3_DPL,
            INTERRUPT_GATE_TYPE);
    install_interrupt_handler(USERMODE_TEST_INTERRUPT,
            usermode_test_interrupt_handler);
}

void register_usermode_monitor_commands(void) {
    monitor_register_command("usermode", usermode_command_enter,
            "enter ring 3 test and return via int 0x81");
}

static int prepare_usermode_test(void) {
    unsigned int code_frame;
    unsigned int stack_frame;

    if (usermode_test_prepared) {
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

    if (!map_user_page(USERMODE_TEST_CODE_VADDR, code_frame)) {
        free_frame(code_frame);
        free_frame(stack_frame);
        return 0;
    }

    if (!map_user_page(USERMODE_TEST_STACK_VADDR, stack_frame)) {
        destroy_page(USERMODE_TEST_CODE_VADDR);
        free_frame(stack_frame);
        return 0;
    }

    write_usermode_test_code();
    usermode_test_prepared = 1;
    return 1;
}

static void write_usermode_test_code(void) {
    unsigned char * code_ptr;
    int i;

    code_ptr = (unsigned char *) USERMODE_TEST_CODE_VADDR;
    for (i = 0; i < (int)sizeof(usermode_test_code); i++) {
        code_ptr[i] = usermode_test_code[i];
    }
}

static void enter_usermode_test(void) {
    unsigned int user_code;
    unsigned int user_data;

    user_code = user_code_selector | 0x03;
    user_data = user_data_selector | 0x03;

    serial_printf("[usermode] entering ring3 code=0x%x stack_top=0x%x int=0x%x\n",
            USERMODE_TEST_CODE_VADDR, USERMODE_TEST_STACK_TOP,
            USERMODE_TEST_INTERRUPT);

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
                      "r"(USERMODE_TEST_STACK_TOP),
                      "r"(IF_ENABLE),
                      "r"(user_code),
                      "r"(USERMODE_TEST_CODE_VADDR)
                    : "memory");
}

static void usermode_command_enter(char * args) {
    (void)args;

    if (user_code_selector == 0 || user_data_selector == 0) {
        klog_printf("usermode: user segments are not initialized\n");
        return;
    }

    if (kernel_tss_selector == 0) {
        klog_printf("usermode: tss is not initialized\n");
        serial_printf("[usermode] missing tss\n");
        return;
    }

    if (!prepare_usermode_test()) {
        klog_printf("usermode: could not prepare user pages\n");
        serial_printf("[usermode] prepare failed\n");
        return;
    }

    klog_printf("usermode: launching first ring 3 test\n");
    klog_printf("usermode: expected flow => int 0x81 returns control to kernel\n");
    enter_usermode_test();

    klog_printf("usermode: unexpected return to monitor command path\n");
}

static void usermode_test_interrupt_handler(interrupt_state * state) {
    usermode_test_interrupt_count++;

    klog_printf("usermode: returned to kernel via int 0x81\n");
    klog_printf("usermode: old_cs=0x%x old_eip=0x%x old_ss=0x%x old_esp=0x%x count=%u\n",
            state->old_cs, state->old_eip, state->old_ss, state->old_esp,
            usermode_test_interrupt_count);
    serial_printf("[usermode] returned via int=0x%x old_cs=0x%x old_eip=0x%x old_ss=0x%x old_esp=0x%x count=%u\n",
            state->number, state->old_cs, state->old_eip, state->old_ss,
            state->old_esp, usermode_test_interrupt_count);

    klog_printf("usermode: ring 3 transition verified; kernel halted intentionally\n");
    serial_printf("[usermode] halting after successful ring3 test\n");
    for (;;) {
        inline_assembly("cli");
        inline_assembly("hlt");
    }
}
