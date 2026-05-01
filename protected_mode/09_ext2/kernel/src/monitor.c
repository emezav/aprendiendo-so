/**
 * @file
 * @ingroup kernel_code
 * @brief Implementacion del dispatcher minimo del monitor del kernel.
 */
#include <console.h>
#include <klog.h>
#include <monitor.h>
#include <string.h>

#define MONITOR_PROMPT "k> "

/**
 * @brief Entrada interna de la tabla de comandos del monitor.
 */
typedef struct monitor_command_entry {
    /** @brief Nombre del comando registrado. */
    char name[MONITOR_MAX_COMMAND_NAME];
    /** @brief Texto corto de ayuda asociado al comando. */
    char help[MONITOR_MAX_HELP_TEXT];
    /** @brief Callback que implementa la accion del comando. */
    monitor_callback callback;
} monitor_command_entry;

/** @brief Tabla estatica de comandos disponibles en el monitor. */
static monitor_command_entry monitor_commands[MONITOR_MAX_COMMANDS];
/** @brief Cantidad actual de comandos registrados. */
static int monitor_command_count;

/**
 * @brief Avanza sobre los espacios iniciales de una cadena.
 * @param text Cadena a procesar.
 * @return Apuntador al primer caracter no espacio o al nulo final.
 */
static char * monitor_skip_spaces(char * text);

/**
 * @brief Inicializa la infraestructura interna del monitor.
 */
void setup_monitor(void) {
    monitor_command_count = 0;
    memset(monitor_commands, 0, sizeof(monitor_commands));
}

/**
 * @brief Registra un comando y su callback en la tabla del monitor.
 * @param name Nombre textual del comando.
 * @param callback Funcion a ejecutar al invocar el comando.
 * @param help Texto corto de ayuda mostrado por `monitor_print_help()`.
 * @return 1 si el registro fue exitoso, 0 en caso contrario.
 */
int monitor_register_command(const char * name, monitor_callback callback,
        const char * help) {
    int i;

    if (name == 0 || callback == 0 || name[0] == 0) {
        return 0;
    }

    if (monitor_command_count >= MONITOR_MAX_COMMANDS) {
        return 0;
    }

    for (i = 0; i < monitor_command_count; i++) {
        if (strcmp(monitor_commands[i].name, name) == 0) {
            return 0;
        }
    }

    strcpy(monitor_commands[monitor_command_count].name, name);
    monitor_commands[monitor_command_count].callback = callback;

    if (help != 0) {
        strcpy(monitor_commands[monitor_command_count].help, help);
    } else {
        monitor_commands[monitor_command_count].help[0] = 0;
    }

    monitor_command_count++;
    return 1;
}

/**
 * @brief Procesa una linea de entrada y despacha el comando correspondiente.
 * @param line Linea completa ingresada por el usuario.
 */
void monitor_handle_line(char * line) {
    char command[MONITOR_MAX_COMMAND_NAME];
    char * args;
    int consumed;
    int i;

    if (line == 0) {
        return;
    }

    line = monitor_skip_spaces(line);
    if (*line == 0) {
        return;
    }

    consumed = nexttok(line, command, ' ', 0);
    if (consumed <= 0) {
        return;
    }

    args = line + consumed;
    args = monitor_skip_spaces(args);

    serial_printf("[monitor] command=\"%s\"\n", line);

    for (i = 0; i < monitor_command_count; i++) {
        if (strcmp(command, monitor_commands[i].name) == 0) {
            monitor_commands[i].callback(args);
            return;
        }
    }

    klog_printf("Unknown command: %s\n", line);
    console_printf("Type 'help' for available commands.\n");
}

/**
 * @brief Imprime el prompt visible del monitor del kernel.
 */
void monitor_print_prompt(void) {
    console_printf(MONITOR_PROMPT);
}

/**
 * @brief Imprime la ayuda de todos los comandos registrados.
 */
void monitor_print_help(void) {
    int i;

    console_printf("Available commands:\n");
    for (i = 0; i < monitor_command_count; i++) {
        if (monitor_commands[i].help[0] != 0) {
            console_printf("  %s - %s\n", monitor_commands[i].name,
                    monitor_commands[i].help);
        } else {
            console_printf("  %s\n", monitor_commands[i].name);
        }
    }
}

/**
 * @brief Retorna la subcadena ubicada despues de los espacios iniciales.
 * @param text Cadena a procesar.
 * @return Primer caracter no espacio de la cadena.
 */
static char * monitor_skip_spaces(char * text) {
    while (text != 0 && *text == ' ') {
        text++;
    }
    return text;
}
