/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Este archivo implementa las rutinas necesarias para el manejo del
 * Programmable Interrupt Timer (PIT) 8253/8254.
 */

#include <asm.h>
#include <console.h>
#include <irq.h>
#include <timer.h>
#include <stdlib.h>
#include <task.h>

/** @brief El PIT oscila por defecto a 1193180 (0x1234dc) Hz. */
#define CLOCK_TICK_RATE 0x1234DC

/** @brief Puerto para el contador 0 del timer (reloj) */
#define TIMER_COUNTER_0_PORT 0x40

/** @brief Puerto de control del modo del timer */
#define TIMER_CONTROL_PORT 0x43

/**
 * @brief  Palabra de control para inicializar el timer
 *  0x34 = 00110100 (Select counter 0, read LSB first then MSB, Mode 2,
 *  binary counter 16-bits
 */
#define TIMER_CONTROL_WORD 0x34

/** @brief Macro para calcular el divisor de frecuencia */
#define TIMER_DIVISOR(x)  (CLOCK_TICK_RATE / x)

/** @brief Número de ticks por segundo = frecuencia del timer */
#define TICKS_PER_SEC TIMER_FREQUENCY

/**
 * @brief Manejador temporal para calibrar el timer
 */
void calibrate_timer_handler(interrupt_state *);

/**
 * @brief Esta rutina se ejecuta cada vez que se recibe la IRQ0
 * (interrupcion 32), que corresponde al timer.
 */
void timer_handler(interrupt_state *);

/**
 * @brief Esta rutina realiza un ciclo simple. Se usa para calibrar
 * la velocidad aproximada del procesador.
 */
void delay_loop(unsigned int );

/** @brief Tabla de funciones que se invocan en cada tick (callbacks)*/
timer_callback timer_callbacks[TIMER_CALLBACK_LIMIT];

/** @brief Numero de callbacks de timer registrados. */
int timer_callback_count = 0;

/**
 * @brief Variable para almacenar el divisor de frecuencia configurado en el
 * timer.*/
unsigned short timer_divisor = 0;

/**
 * @brief Variable para almacenar la frecuencia configurada en el timer, por
 * defecto 100 hz = 10 interrupciones por segundo
 */
unsigned short timer_frequency = 0;

/** @brief Variable que almacena el numero de ticks (interrupciones de reloj)*/
unsigned int timer_ticks = 0;

/**
 * @brief Variable que se utiliza para determinar el numero de veces que se
 * puede ejecutar la funcion delay_loop en un tick de reloj. Su valor
 * es determinado en la rutina calibrate_timer
 */
unsigned int loops_per_tick;

/**
 * @brief Cantidad de veces que se puede ejecutar la funcion delay_loop en un
 * segundo. Depende del valor de loops_per_tick.
 */
unsigned int loops_per_sec;

/** @brief Cantidad de veces que se puede ejecutar la funcion delay_loop en un
 * milisegundo. Depende del valor de loops_per_tick */
unsigned int loops_per_msec;

/**
 * @brief Cantidad de veces que se puede ejecutar la funcion delay_loop en un
 * microsegundo. Depende del valor de loops_per_tick
 */
unsigned int loops_per_usec;

/**
 * @brief Cantidad de veces que se puede ejecutar la funcion delay_loop en un
 * nanosegundo. Depende del valor de loops_per_tick
 */
unsigned int loops_per_nsec;

/* Suponer que la rutina delay_loop se puede ejecutar por lo menos
 * una vez en cada precision. Dentro de la funcion calibrate_timer se establecen
 * los valores reales para cada precision. */

/** @brief Precisión en milisegundos */
int msec_precision = 1;

/** @brief Precisión en microsegundos */
int usec_precision = 1;

/** @brief Precisión en nanosegundos */
int nsec_precision = 1;

/**
 * @brief Esta rutina determina el numero aproximado de veces que
 * se puede ejecutar la rutina 'delay_loop' en un tick de reloj.
 * Con ello se actualiza la variable global 'loops_per_tick'
 */
void calibrate_timer();

/**
 * @brief Esta rutina configura la frecuencia con la cual el timer
 * lanzara la interrupcion, y ademas define la rutina de manejo de interrupcion
 * del timer.
 */
void setup_timer() {

  /** Configuración del timer
  * Primero se establece la frecuencia con la cual el timer lanzará la
  * interrupcion */

  /** - Establecer la variable global de la frecuencia del timer */
  timer_frequency = TIMER_FREQUENCY;

  /** - Calcular el valor del divisor del timer  */
  timer_divisor = TIMER_DIVISOR(timer_frequency);

  /** - Enviar  0x34 = 00110100  al puerto de comandos del PIT (0x43) */
  outb(TIMER_CONTROL_PORT, TIMER_CONTROL_WORD);

  /** - Enviar el byte menos significativo del divisor al puerto 0x40
   * (counter 0) del pit */
  outb(TIMER_COUNTER_0_PORT,  timer_divisor & 0xFF);

  /** - Enviar el byte mas significativo del divisor al puerto 0x40 (counter 0)
   * del pit */
  outb(TIMER_COUNTER_0_PORT,( timer_divisor >> 8 ) & 0xFF);

  /** - Configurar la rutina definitiva de manejo de interrupción del timer. */
  install_irq_handler(TIMER_IRQ, calibrate_timer_handler);

  /* Importante!! se asume que las interrupciones estan deshabilitadas
   * en este punto.*/
  inline_assembly("sti");

  /* Calibrar el timer */
  calibrate_timer();

  /* Deshabilitar de nuevo las interrupciones */
  inline_assembly("cli");

  /* Desinstalar el manejador temporal */
  uninstall_irq_handler(TIMER_IRQ);

  /* Inicializar la estructura de datos de callbacks */
  memset(timer_callbacks, 0, sizeof(timer_callbacks));

  timer_callback_count = 0;

  //console_printf("Installing timer interrupt handler\n");
  /** - Configurar la rutina definitiva de manejo de interrupción del timer. */
  install_irq_handler(TIMER_IRQ, timer_handler);

}


/** @brief Instala una rutina (callback) a ser invocada en cada tick.*/
void add_timer_callback(timer_callback callback) {
    if (timer_callback_count == TIMER_CALLBACK_LIMIT) {
        console_printf("No hay espacio para registrar un nuevo callback de "
                "timer\n");
        for (;;);
    }

    timer_callbacks[timer_callback_count++] = callback;
}

/**
 * @brief Retorna el valor del contador de ticks de reloj del kernel.
 * 	@return Número de ticks de reloj desde el arranque del sistema
 */
unsigned int current_tick() {
	return timer_ticks;
}

/**
 * @brief Manejador temporal para la IRQ del timer.
 * @param state Estructura de datos que contiene el estado de los registros
 * 				del procesador en el momento de la interrupción del timer
 */
void calibrate_timer_handler(interrupt_state * state) {
	timer_ticks++;
}

/**
 * @brief Esta funcion recibe el control de la rutina de manejo
 * de interrupcion del timer. Cada vez que se ejecuta, aumenta en 1 el contador
 * para el numero de ticks (interrupciones de reloj).
 * @param state Estructura de datos que contiene el estado de los registros
 * 				del procesador en el momento de la interrupción del timer
 */
void timer_handler(interrupt_state * state) {
    int i;

    /** - Incrementar el numero de ticks de reloj. */
	timer_ticks++;

    /** - Invocar los callbacks de timer instalados. */
    for (i = 0; i < timer_callback_count; i++) {
        timer_callbacks[i](timer_ticks);
    }

    /** - Invocar el planificador si la multitarea se encuentra lista.*/
    if (multitasking_ready) {
        task_dispatcher();
    }
}

/**
 * @brief Esta rutina realiza un ciclo simple. Se usa para calibrar
 * la velocidad aproximada del procesador.
 * @param loops Número de ciclos a ejecutar
 */
void delay_loop(unsigned int loops) {
    int i;
    for (i = 0; i < loops; i++);
}

/**
 * @brief Esta rutina determina el número aproximado de veces que
 * se puede ejecutar la rutina 'delay_loop' en un tick de reloj.
 * Con ello se actualiza la variable global 'loops_per_tick'.
 */
void calibrate_timer() {
	unsigned int start_tick;
	unsigned int end_tick;
	unsigned int remainder;

	/** - Determinar la mayor potencia de 2 de loops_per_tick.
	 * Máximo: 32 iteraciones para un procesador de 32 bits
	 * Se inicia con 1 = 2^0 */
	loops_per_tick = 1;

	do {
		/**    - Esperar al inicio de un tick */
		start_tick = timer_ticks;
		while (start_tick == timer_ticks);

		/**    - Medir desde el inicio del tick */
		start_tick = timer_ticks;

		/**    - Ejecutar la funcion auxiliar delay_loop */
		delay_loop(loops_per_tick);

		/**    - Medir el fin del tick */
		end_tick = timer_ticks;

		/**    - Verificar si ejecutar la rutina delay_loop loops_per_tick veces
		 * toma mas de un tick */
		if (start_tick != end_tick) {
			break;
		}
	}while ((loops_per_tick *= 2)!=0 );

	/** - Tomar la anterior potencia de 2, que es la que se aproxima por debajo
	 * al número de loops por tick */
	loops_per_tick /= 2;

	/** - Tratar de aproximar loops_per_tick sumando potencias de
	 * 2 menores o iguales a loops_per_tick / 2 */
	remainder = loops_per_tick / 2;

	do {
		loops_per_tick += remainder;
		/**    - Esperar al inicio de un tick */
		start_tick = timer_ticks;
		while (start_tick == timer_ticks);

		/**    - Medir desde el inicio del tick */
		start_tick = timer_ticks;

		/**    - Ejecutar la funcion auxiliar delay_loop */
		delay_loop(loops_per_tick);

		/**    - Medir el fin del tick */
		end_tick = timer_ticks;

		/**    - Verificar si ejecutar la rutina delay_loop loops_per_tick veces
				 * toma mas de un tick */
		if (start_tick != end_tick) {
			loops_per_tick -= remainder;
		}

		/** - Buscar la siguiente potencia de 2. */
		remainder /= 2;
	}while (remainder > 0);

	/* loops_per_tick contiene el valor aproximado del numero de veces
	 * que se puede ejecutar la funcion delay_loop en un tick de reloj. */

	//console_printf("loops per tick: %u\n", loops_per_tick);

	/** - Calcular los valores del numero de loops por segundo,
	 *  por milisegundo, microsegundo y nanosegundo. Tambien se debe
	 *  calcular la resolucion minima para cada unidad. */

	loops_per_sec =	TICKS_PER_SEC * loops_per_tick;

	/* La funcion delay_loop se deberia ejecutar al menos una vez en un
	 * segundo, ya que TICKS_PER_SEC es mayor que 1 */
	if (loops_per_sec == 0) {
		console_printf(" Error! cannot calculate loops per second!");
		return;
	}

	loops_per_msec = loops_per_sec / 1000;

	/* No alcanza a ejecutarse un loop en un milisegundo? */
	if (loops_per_msec == 0) {
		msec_precision =loops_per_sec / 100;
		if (msec_precision == 0) {
			msec_precision =loops_per_sec / 10;
			/* No se puede calcular la resolucion en milisegundos*/
			if (msec_precision == 0) {
				msec_precision = 1000;
			}else {
				msec_precision++;
			}
		}else {
			msec_precision++;
		}
	}
	//console_printf("milisec precision: %d\n", msec_precision);

	loops_per_usec = loops_per_sec / 1000000;

	/* No alcanza a ejecutarse un loop en un microsegundo? */
	if (loops_per_usec == 0) {
		usec_precision = loops_per_sec / 100000;
		if (usec_precision == 0) {
			usec_precision = loops_per_sec / 10000;
			if (usec_precision == 0) {
				usec_precision = 1000;
			}else {
				usec_precision++;
			}
		}else {
			usec_precision++;
		}
	}
	//console_printf("microsec precision: %d\n", msec_precision);

	loops_per_nsec = loops_per_sec / 1000000000;

	/* No alcanza a ejecutarse un loop en un nanosegundo? */
	if (loops_per_nsec == 0) {
		nsec_precision = loops_per_sec / 100000000;
		if (nsec_precision == 0) {
			nsec_precision = loops_per_sec / 10000000;
			if (nsec_precision == 0) {
				nsec_precision = 1000;
			}else {
				nsec_precision++;
			}
		}else {
			nsec_precision++;
		}
	}
	//console_printf("nanosec precision: %d\n", nsec_precision);

}

/**
 * @brief Esta rutina permite realizar demoras en un numero determinado
 * de segundos. Para realizar la demora, usa la funcion auxiliar delay_loop,
 * que realiza una espera activa. Su precision depende del calculo del numero de
 * veces que se puede ejecutar la funcion delay_loop en un tick de reloj. Debido
 * a su baja precision, se limita a esperas menores o iguales a 256.
 * @param secs Numero de segundos (0-255)
 */
void sdelay(unsigned char secs) {
	unsigned int loops;

	/* Calcular el numero de loops que se deben ejecutar */
	loops = secs * TICKS_PER_SEC; /* segundos * (ticks/segundo) */

	while (loops > 0) { /* Iterar el numero de ticks necesario */
		delay_loop(loops_per_tick); /* Demorarse un tick */
		loops--;
	}
}

/**
 * @brief Esta rutina permite realizar demoras en un numero determinado
 * de milisegundos. Para realizar la demora, usa la funcion auxiliar delay_loop,
 * que realiza una espera activa. Su precision depende del calculo del numero de
 * veces que se puede ejecutar la funcion delay_loop en un tick de reloj.
 * @param milisecs Numero de milisegundos
 */
void delay(unsigned int milisecs){

	int i;
	unsigned int loops;

	if (milisecs >= 1000) {
		sdelay(milisecs / 1000);
		milisecs = milisecs % 1000;
	}

	if (milisecs == 0) {return;}
	if (milisecs < msec_precision) {
		milisecs = msec_precision;
	}
	/* Calcular el numero de ciclos que se deben ejecutar en los
	 * milisegundos especificados */


	/* Calcular el numero de loops que se deben ejecutar */
	loops = (milisecs + msec_precision) * loops_per_msec;

	delay_loop(loops); // Demorarse el numero de loops calculados

}

/**
 * @brief Esta rutina permite realizar demoras en un numero determinado
 * de microsegundos. Para realizar la demora, usa la funcion auxiliar
 * delay_loop, que realiza una espera activa. Su precision depende del calculo
 * del numero de veces que se puede ejecutar la funcion delay_loop en un tick de
 * reloj.
 * @param usecs Número de milisegundos a esperar
 */
void udelay(unsigned int usecs){

	int i;
	unsigned int loops;

	if (usecs >= 1000) {
		delay(usecs / 1000);
		usecs = usecs % 1000;
	}

	if (usecs == 0) {return;}
	if (usecs < usec_precision) {
		usecs = usec_precision;
	}
	/* Calcular el numero de ciclos que se deben ejecutar en los
	 * microsegundos especificados */


	loops = (usecs  + usec_precision) * loops_per_usec;

	delay_loop(loops); // Demorarse el numero de loops calculados

}

/**
 * @brief Esta rutina permite realizar demoras en un numero determinado
 * de nanosegundos. Para realizar la demora, usa la funcion auxiliar
 * delay_loop, que realiza una espera activa. Su precision depende del calculo
 * del numero de veces que se puede ejecutar la funcion delay_loop en un tick de
 * reloj.
 * @param nsecs Número de nanosegundos
 */
void ndelay(unsigned int nsecs){

	int i;
	unsigned int loops;

	if (nsecs >= 1000) {
		udelay(nsecs / 1000);
		nsecs = nsecs % 1000;
	}

	if (nsecs == 0) {return;}
	if (nsecs < nsec_precision) {
		nsecs = nsec_precision;
	}
	/* Calcular el numero de ciclos que se deben ejecutar en los
	 * nanosegundos especificados */


	loops = (nsecs + nsec_precision) * loops_per_nsec ;
	//console_printf("Loops: %u lpt: %u", loops, loops_per_tick);

	delay_loop(loops); // Demorarse el numero de loops calculados

}
