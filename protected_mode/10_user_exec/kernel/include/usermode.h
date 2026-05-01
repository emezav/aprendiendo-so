/**
 * @file
 * @ingroup kernel_code
 * @brief Infraestructura minima para la primera prueba controlada de userland.
 */

#ifndef USERMODE_H_
#define USERMODE_H_

/**
 * @brief Configura la interrupcion de prueba accesible desde `ring 3`.
 */
void setup_usermode(void);

/**
 * @brief Registra en el monitor los comandos de prueba de userland.
 */
void register_usermode_monitor_commands(void);

#endif /* USERMODE_H_ */
