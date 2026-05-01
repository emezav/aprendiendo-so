/**
 * @file
 * @ingroup kernel_code
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License.
 *
 * @brief Definiciones publicas para el timer del sistema.
 */

#ifndef TIMER_H_
#define TIMER_H_

/** @brief Frecuencia base del PIT 8253/8254. */
#define PIT_BASE_FREQUENCY 1193180

/** @brief Puerto de datos del canal 0 del PIT. */
#define PIT_CHANNEL0_PORT 0x40

/** @brief Puerto de comando del PIT. */
#define PIT_COMMAND_PORT 0x43

/** @brief Frecuencia por defecto para el timer del sistema. */
#define TIMER_DEFAULT_HZ 100

/**
 * @brief Inicializa el timer del sistema.
 *
 * Esta rutina configura el PIT para generar interrupciones periodicas por la
 * IRQ0 e instala el manejador asociado.
 */
void setup_timer(void);

/**
 * @brief Configura la frecuencia del timer del sistema.
 * @param hz Frecuencia deseada en Hertz.
 */
void timer_set_frequency(unsigned int hz);

/**
 * @brief Obtiene la cantidad de ticks atendidos por el timer.
 * @return Numero de ticks acumulados.
 */
unsigned int timer_get_ticks(void);

/**
 * @brief Obtiene la frecuencia actual configurada para el timer.
 * @return Frecuencia del timer en Hertz.
 */
unsigned int timer_get_frequency(void);

#endif /* TIMER_H_ */
