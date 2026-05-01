/**
 * @file
 * @ingroup kernel_code
 * @brief Infraestructura minima para registrar y despachar comandos del
 * monitor del kernel.
 */

#ifndef MONITOR_H_
#define MONITOR_H_

#define MONITOR_MAX_COMMANDS 256
#define MONITOR_MAX_COMMAND_NAME 32
#define MONITOR_MAX_HELP_TEXT 64

/**
 * @brief Tipo de callback invocado por el monitor para un comando registrado.
 * @param args Cadena con los argumentos crudos del comando. Puede ser vacia.
 */
typedef void (*monitor_callback)(char * args);

/**
 * @brief Inicializa la tabla interna de comandos del monitor.
 */
void setup_monitor(void);

/**
 * @brief Registra un comando en el monitor.
 * @param name Nombre del comando.
 * @param callback Funcion a invocar cuando el comando es ejecutado.
 * @param help Texto corto de ayuda para listar el comando.
 * @return 1 si el registro fue exitoso, 0 en caso contrario.
 */
int monitor_register_command(const char * name, monitor_callback callback,
        const char * help);

/**
 * @brief Ejecuta la linea ingresada en el monitor.
 * @param line Linea completa de entrada.
 */
void monitor_handle_line(char * line);

/**
 * @brief Imprime el prompt del monitor.
 */
void monitor_print_prompt(void);

/**
 * @brief Imprime la lista de comandos registrados.
 */
void monitor_print_help(void);

#endif /* MONITOR_H_ */
