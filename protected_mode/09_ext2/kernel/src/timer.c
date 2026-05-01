/**
 * @file
 * @ingroup kernel_code
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License.
 *
 * @brief Implementacion del timer del sistema usando el PIT.
 */

#include <asm.h>
#include <irq.h>
#include <pm.h>
#include <serial.h>
#include <timer.h>

/** @brief Contador global de ticks del sistema. */
static unsigned int timer_ticks = 0;

/** @brief Frecuencia actual del timer en Hertz. */
static unsigned int timer_hz = TIMER_DEFAULT_HZ;

/**
 * @brief Manejador privado de la IRQ0 del timer.
 * @param state Marco de pila de la interrupcion.
 */
static void timer_handler(interrupt_state * state);

void setup_timer(void) {
    timer_ticks = 0;
    timer_set_frequency(TIMER_DEFAULT_HZ);
    install_irq_handler(0, timer_handler);
}

void timer_set_frequency(unsigned int hz) {
    unsigned int divisor;

    if (hz == 0) {
        hz = TIMER_DEFAULT_HZ;
    }

    divisor = PIT_BASE_FREQUENCY / hz;

    if (divisor == 0) {
        divisor = 1;
    }

    timer_hz = hz;

    /* Canal 0, acceso low/high, modo 3, contador binario. */
    outb(PIT_COMMAND_PORT, 0x36);
    outb(PIT_CHANNEL0_PORT, divisor & 0xFF);
    outb(PIT_CHANNEL0_PORT, (divisor >> 8) & 0xFF);
}

unsigned int timer_get_ticks(void) {
    return timer_ticks;
}

unsigned int timer_get_frequency(void) {
    return timer_hz;
}

/**
 * @brief Atiende cada tick del PIT en la etapa de consola por IRQ.
 * @param state Marco de interrupcion asociado a la IRQ0 actual.
 *
 * En `04_irq_console` el timer se usa para validar que las IRQ periodicas
 * conviven correctamente con el teclado y con el lazo principal del kernel.
 */
static void timer_handler(interrupt_state * state) {
    (void) state;
    timer_ticks++;

    if (timer_hz != 0 && (timer_ticks % timer_hz) == 0) {
        serial_printf("[timer] tick=%u\n", timer_ticks);
    }
}
