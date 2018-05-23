/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Contiene las definiciones necesarias para el manejo del
 * Programmable Interrupt Timer (PIT) 8253/8254.
 */

#ifndef TIMER_H_
#define TIMER_H_

/** @brief Numero de IRQ para el timer: En la arquitectura IA-32 le corresponde
 * la IRQ 0*/
#define TIMER_IRQ 0

/**
 * @brief Frecuencia por defecto en la cual se programa el PIT. Este es el
 * número de ticks de reloj (IRQ, interrupciones) que se generan en un
 * segundo */
#define TIMER_FREQUENCY 1000 /* ticks / segundo*/

/* @brief Tipo de datos de las rutinas que se invocan en cada tick de reloj*/
typedef void (*timer_callback)(unsigned int);

/** @brief Numero maximo de callbacks de timer que se pueden manejar */
#define TIMER_CALLBACK_LIMIT 256

/**
 * @brief Esta rutina configura la frecuencia con la cual el timer
 * lanzara la interrupcion, y ademas define la rutina de manejo de IRQ
 * del timer.
 */
void setup_timer();

/** @brief Instala una rutina (callback) a ser invocada en cada tick.*/
void add_timer_callback(timer_callback);

/**
 * @brief Retorna el valor del contador de ticks de reloj del kernel.
 * @return Número de ticks de reloj desde el arranque del sistema
 */
unsigned int current_tick();

/**
 * @brief Esta rutina permite realizar demoras en un numero determinado
 * de segundos. Para realizar la demora, usa la funcion auxiliar delay_loop,
 * que realiza una espera activa. Su precision depende del calculo del numero de
 * veces que se puede ejecutar la funcion delay_loop en un tick de reloj. Debido
 * a su baja precision, se limita a esperas menores o iguales a 256.
 * @param secs Numero de segundos (0-255)
 */
void sdelay(unsigned char);

/**
 * @brief Esta rutina permite realizar demoras en un numero determinado
 * de milisegundos. Para realizar la demora, usa la funcion auxiliar delay_loop,
 * que realiza una espera activa. Su precision depende del calculo del numero de
 * veces que se puede ejecutar la funcion delay_loop en un tick de reloj.
 * @param milisecs Numero de milisegundos
 */
void delay(unsigned int);

/**
 * @brief Esta rutina permite realizar demoras en un numero determinado
 * de microsegundos. Para realizar la demora, usa la funcion auxiliar
 * delay_loop, que realiza una espera activa. Su precision depende del calculo
 * del numero de veces que se puede ejecutar la funcion delay_loop en un tick de
 * reloj.
 * @param usecs Número de milisegundos a esperar
 */
void udelay(unsigned int);

/**
 * @brief Esta rutina permite realizar demoras en un numero determinado
 * de nanosegundos. Para realizar la demora, usa la funcion auxiliar
 * delay_loop, que realiza una espera activa. Su precision depende del calculo
 * del numero de veces que se puede ejecutar la funcion delay_loop en un tick de
 * reloj.
 * @param nsecs Número de nanosegundos
 */
void ndelay(unsigned int);


#endif /* TIMER_H_ */
