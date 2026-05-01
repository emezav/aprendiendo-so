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
#include <irq.h>
#include <keyboard.h>
#include <klog.h>
#include <kmem.h>
#include <paging.h>
#include <pci.h>
#include <pm.h>
#include <physmem.h>
#include <serial.h>
#include <stdlib.h>
#include <string.h>
#include <timer.h>

#define KMONITOR_PROMPT "k> "
#define KMONITOR_MAX_LINE 128

/**
 * @brief Monitor de comandos basico para el kernel.
 */
static void kernel_monitor_print_prompt(void);

/**
 * @brief Borra el ultimo caracter impreso en la consola. Se utiliza para implementar
 * el comportamiento de la tecla de retroceso (backspace) en el monitor de comandos.
 *
 */
static void kernel_monitor_erase_last_char(void);

/**
 * @brief Ejecuta un comando ingresado en el monitor de comandos del kernel.
 * @param line La linea de texto que contiene el comando a ejecutar.
 *
 * Este monitor de comandos soporta los siguientes comandos:
 * - help: Muestra un mensaje con los comandos disponibles.
 * - ticks: Muestra el numero de ticks del timer del sistema.
 * - clear: Limpia la pantalla de la consola.
 * - halt: Detiene el kernel deshabilitando interrupciones y ejecutando `hlt`.
 *
 * Si se ingresa un comando desconocido, se muestra un mensaje indicando que el comando no es reconocido.
 */
static void kernel_monitor_run_command(char * line);

/**
 * @brief Punto de entrada principal del kernel en C.
 *
 * Esta rutina completa la inicializacion de los subsistemas del kernel una vez
 * que `start.S` transfiere el control al codigo C y luego mantiene el monitor
 * minimo basado en teclado, timer y consola.
 */
void cmain(){
    char c;
    char line_buffer[KMONITOR_MAX_LINE];
    int line_length;

    /* Inicializar el puerto serial lo antes posible para depuracion. */
    setup_serial();
    serial_printf("[serial] cmain(): starting kernel initialization\n");

    /* Inicializar y limpiar la consola. console.c */
    setup_console();
    serial_printf("[serial] console initialized\n");

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

    /* Las subrutinas que se deben ejecutar DESPUES de habilitar las
     * interrupciones se deben invocar en este punto. */

    /* Inicializar la informacion del bus PCI. pci.c */
    setup_pci();
    serial_printf("[serial] pci initialized\n");

    /* Inicializar el timer del sistema. timer.c */
    setup_timer();
    serial_printf("[serial] timer initialized at %u Hz\n",
            timer_get_frequency());

    /* Inicializar el controlador de teclado. keyboard.c */
    setup_keyboard();
    serial_printf("[serial] keyboard initialized\n");

    /* Imprimir mensajes de arranque. */
    klog_printf("Kernel started.\n");
    serial_printf("[serial] kernel started\n");
    line_length = 0;
    memset(line_buffer, 0, sizeof(line_buffer));
    kernel_monitor_print_prompt();

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
                kernel_monitor_run_command(line_buffer);
                line_length = 0;
                memset(line_buffer, 0, sizeof(line_buffer));
                kernel_monitor_print_prompt();
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

static void kernel_monitor_print_prompt(void) {
    console_printf(KMONITOR_PROMPT);
}

static void kernel_monitor_erase_last_char(void) {
    console_putchar('\b');
    console_putchar(' ');
    console_putchar('\b');
}

static void kernel_monitor_run_command(char * line) {
    if (line == 0 || line[0] == 0) {
        return;
    }

    serial_printf("[monitor] command=\"%s\"\n", line);

    if (strcmp(line, "help") == 0) {
        console_printf("Available commands:\n");
        console_printf("  help  - show this message\n");
        console_printf("  ticks - show timer ticks\n");
        console_printf("  clear - clear the screen\n");
        console_printf("  halt  - stop the kernel\n");
        return;
    }

    if (strcmp(line, "ticks") == 0) {
        klog_printf("ticks: %u\n", timer_get_ticks());
        return;
    }

    if (strcmp(line, "clear") == 0) {
        console_clear();
        return;
    }

    if (strcmp(line, "halt") == 0) {
        klog_printf("halting kernel...\n");
        for (;;) {
            inline_assembly("cli");
            inline_assembly("hlt");
        }
    }

    klog_printf("Unknown command: %s\n", line);
    console_printf("Type 'help' for available commands.\n");
}
