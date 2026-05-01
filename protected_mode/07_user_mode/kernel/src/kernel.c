/**
 * @file
 * @ingroup kernel_code
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License.
 *
 * @brief Codigo de inicializacion del kernel en C
 *
 * Este codigo recibe el control de start.S y continua con la ejecucion.
*/
#include <asm.h>
#include <console.h>
#include <disk.h>
#include <irq.h>
#include <keyboard.h>
#include <klog.h>
#include <kmem.h>
#include <meminfo.h>
#include <monitor.h>
#include <paging.h>
#include <pci.h>
#include <pm.h>
#include <physmem.h>
#include <serial.h>
#include <stdlib.h>
#include <string.h>
#include <timer.h>
#include <tss.h>
#include <usermode.h>

#define KMONITOR_MAX_LINE 128

/**
 * @brief Registra los comandos basicos del monitor.
 */
static void register_kernel_monitor_commands(void);

/**
 * @brief Ejecuta el bucle principal del monitor del kernel.
 */
void kernel_enter_monitor_loop(void);

/**
 * @brief Borra el ultimo caracter impreso en la consola. Se utiliza para implementar
 * el comportamiento de la tecla de retroceso (backspace) en el monitor de comandos.
 *
 */
static void kernel_monitor_erase_last_char(void);

/**
 * @brief Callback del comando `help`.
 * @param args Argumentos del comando. No se usan en esta version.
 */
static void monitor_command_help(char * args);
/**
 * @brief Callback del comando `ticks`.
 * @param args Argumentos del comando. No se usan en esta version.
 */
static void monitor_command_ticks(char * args);
/**
 * @brief Callback del comando `clear`.
 * @param args Argumentos del comando. No se usan en esta version.
 */
static void monitor_command_clear(char * args);
/**
 * @brief Callback del comando `halt`.
 * @param args Argumentos del comando. No se usan en esta version.
 */
static void monitor_command_halt(char * args);

/**
 * @brief Punto de entrada principal del kernel en C.
 *
 * Esta rutina completa la inicializacion de los subsistemas del kernel una vez
 * que `start.S` transfiere el control al codigo C y luego mantiene el monitor
 * minimo basado en teclado, timer y consola.
 */
void cmain(){
    /* Inicializar el puerto serial lo antes posible para depuracion. */
    setup_serial();
    serial_printf("[serial] cmain(): starting kernel initialization\n");

    /* Inicializar y limpiar la consola. console.c */
    setup_console();
    serial_printf("[serial] console initialized\n");

    /* Inicializar la infraestructura del monitor. */
    setup_monitor();

    /* Inicializar la estructura para gestionar la memoria fisica. physmem.c */
    setup_physical_memory();
    serial_printf("[serial] physical memory initialized\n");

    /* Las subrutinas que se deben ejecutar ANTES de habilitar las
     * interrupciones se deben invocar en este punto. */

    /* Configura la IDT y el PIC. interrupt.c */
    setup_interrupts();
    serial_printf("[serial] interrupts initialized\n");

    /* Completa la configuracion de la memoria virtual. paging.c */
    setup_paging();
    serial_printf("[serial] paging initialized\n");

    /* Configurar la gestion de paginas de memoria virtual. kmem.c */
    setup_kmem();
    serial_printf("[serial] kmem initialized\n");

    /* Reservar y configurar los segmentos minimos para userland. pm.c */
    setup_user_segments();
    serial_printf("[serial] user segments initialized\n");

    /* Configurar un TSS minimo para futuras entradas desde ring 3. */
    setup_tss();
    serial_printf("[serial] tss initialized\n");

    /* Abrir la interrupcion de prueba para la primera entrada a userland. */
    setup_usermode();
    serial_printf("[serial] usermode test interrupt initialized\n");

    /* Las subrutinas que se deben ejecutar DESPUES de habilitar las
     * interrupciones se deben invocar en este punto. */

    /* Inicializar la informacion del bus PCI. pci.c */
    setup_pci();
    serial_printf("[serial] pci initialized\n");

    /* Preparar una ruta minima de lectura real desde disco. */
    setup_disk_io();
    serial_printf("[serial] disk io initialized\n");

    /* Inicializar el timer del sistema. timer.c */
    setup_timer();
    serial_printf("[serial] timer initialized at %u Hz\n",
            timer_get_frequency());

    /* Inicializar el controlador de teclado. keyboard.c */
    setup_keyboard();
    serial_printf("[serial] keyboard initialized\n");

    register_kernel_monitor_commands();
    register_meminfo_monitor_commands();
    register_disk_monitor_commands();
    register_usermode_monitor_commands();

    /* Imprimir mensajes de arranque. */
    klog_printf("Kernel started.\n");
    serial_printf("[serial] kernel started\n");
    kernel_enter_monitor_loop();
}

void kernel_enter_monitor_loop(void) {
    char c;
    char line_buffer[KMONITOR_MAX_LINE];
    int line_length;

    line_length = 0;
    memset(line_buffer, 0, sizeof(line_buffer));
    monitor_print_prompt();

    for (;;) {
        while (keyboard_has_char()) {
            c = keyboard_getchar();

            if (c == '\b') {
                if (line_length > 0) {
                    line_length--;
                    line_buffer[line_length] = 0;
                    kernel_monitor_erase_last_char();
                }
                continue;
            }

            if (c == '\n') {
                console_putchar('\n');
                line_buffer[line_length] = 0;
                monitor_handle_line(line_buffer);
                line_length = 0;
                memset(line_buffer, 0, sizeof(line_buffer));
                monitor_print_prompt();
                continue;
            }

            if (c >= ' ' && c <= '~') {
                if (line_length < (KMONITOR_MAX_LINE - 1)) {
                    line_buffer[line_length++] = c;
                    line_buffer[line_length] = 0;
                    console_putchar(c);
                }
            }
        }
        inline_assembly("hlt");
    }
}

/**
 * @brief Registra los comandos basicos soportados directamente por el kernel.
 */
static void register_kernel_monitor_commands(void) {
    monitor_register_command("help", monitor_command_help,
            "show this message");
    monitor_register_command("ticks", monitor_command_ticks,
            "show timer ticks");
    monitor_register_command("clear", monitor_command_clear,
            "clear the screen");
    monitor_register_command("halt", monitor_command_halt,
            "stop the kernel");
}

static void kernel_monitor_erase_last_char(void) {
    console_putchar('\b');
    console_putchar(' ');
    console_putchar('\b');
}

static void monitor_command_help(char * args) {
    (void)args;
    monitor_print_help();
}

static void monitor_command_ticks(char * args) {
    (void)args;
    klog_printf("ticks: %u\n", timer_get_ticks());
}

static void monitor_command_clear(char * args) {
    (void)args;
    console_clear();
}

static void monitor_command_halt(char * args) {
    (void)args;
    klog_printf("halting kernel...\n");
    for (;;) {
        inline_assembly("cli");
        inline_assembly("hlt");
    }
}
