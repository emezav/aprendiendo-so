/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Este archivo implementa las primitivas necesarias para el manejo
 * de excepciones en la arquitectura IA-32.
 */

#include <console.h>
#include <exception.h>
#include <pm.h>


/** Estructura de datos para almacenar las rutinas que manejaran las
 * excepciones
 */
exception_handler exception_handlers[MAX_EXCEPTIONS];

/**
 * @brief Recibe el control de la rutina interrupt_dispatcher.
 * Su trabajo consiste en determinar el vector de interrupcion a partir del
 * estado que recibe como parametro, y de invocar la rutina de manejo de
 * excepcion adecuada, si existe.
 */
void exception_dispatcher();

/** @brief Excepciones del procesador IA-32 */
unsigned char *exceptions[] = {
	"Division By Zero",
	"Debug",
	"Non Maskable Interrupt",
	"Breakpoint",
	"Into Detected Overflow",
	"Out of Bounds",
	"Invalid Opcode",
	"No Coprocessor",
	"Double Fault",
	"Coprocessor Segment Overrun",
	"Bad TSS",
	"Segment Not Present",
	"Stack Fault",
	"General Protection Fault",
	"Page Fault",
	"Unknown Interrupt",
	"Coprocessor Fault",
	"Alignment Check",
	"Machine Check",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};

/**
 * @brief Crea un manejador de interrupción para las
 * 32 excepciones de IA-32 e inicializa la tabla de manejadores de excepción.
 * Para todas las excepciones se establece la rutina 'exception_dispatcher'
 * como la rutina de manejo de interrupción.
 * Esta rutina se encarga de "despachar" a un manejador de la excepción,
 * configurado para cada una de ellas mediante la función
 * install_exception_handler().
 */
void setup_exceptions(void) {
	int i;

	for (i=0; i<MAX_EXCEPTIONS; i++) {
		exception_handlers[i] = NULL_INTERRUPT_HANDLER;
	}

	/* En la arquitectura Intel las excepciones tienen vectores (numero) 0..31
	 * Asi, el vector es el mismo numero de la excepcion*/

	for (i=0; i<MAX_EXCEPTIONS; i++) {
		install_interrupt_handler(i, exception_dispatcher);
	}
}


/**
 * @brief Recibe el control de la interrupt_dispatcher.
 * Su trabajo consiste en determinar el vector de interrupcion a partir del
 * contexto actual de interrupcion, y de invocar la rutina de manejo de
 * excepcion adecuada, si existe.
 */
void exception_dispatcher() {
	/* Esta variable definida en start.S contiene el valor del apuntador
	 * actual al tope de la pila (esp)*/
	extern unsigned int current_esp;

	extern void dump_interrupt_state(interrupt_state *);

	interrupt_state * state;

	state = (interrupt_state *)current_esp;

	exception_handler handler;

	/* Buscar la rutina que maneja la excepcion. El numero
	 * de la interrupcion (0..31) es el mismo sub-indice en la
	 * tabla de manejadores de excepcion*/
	handler = exception_handlers[state->number];

	/* Si la rutina existe, ejecutarla y pasarle como parametro los registros.*/
	if (handler != NULL_INTERRUPT_HANDLER) {
			handler(state);
	} else {
		/* En caso contrario, informar que ocurrio una interrupcion
		 * que no tiene un manejador asociado.*/
		console_printf("x86 Exception [%d]: %s. System Halted!\n", state->number,
				exceptions[state->number]);

        dump_interrupt_state(state);

		/* Bloquear el kernel cuando ocurre una excepcion que no tiene
		 * manejador asociado.
		 * Recuerde que esta rutina se esta ejecutando con las interrupciones
		 * deshabilitadas, por lo cual no existe forma de romper el
		 * ciclo infinito que se define a continuacion.*/

		for (;;)
			;
	}
}

/**
 * @brief Permite definir un nuevo manejador de excepcion
 * para una de las excepciones de los procesadores x86.
 * @param index Número de la excepción a la cual se le desea instalar
 * la rutina de manejo
 * @param handler Función de manejo de la excepción
 */
int install_exception_handler(unsigned char index, exception_handler handler) {
	if (exception_handlers[index] != NULL_INTERRUPT_HANDLER) {
		console_printf("Error! handler for exception %d was already set!\n", index);
		return -1;
	}
	exception_handlers[index] = handler;
	return index;
}

/**
 * @brief Permite quitar un  manejador de excepcion
 * @param index Número de la excepción a la cual se desea desinstalar
 * su manejador
 */
void uninstall_exception_handler(unsigned char index) {
	/* Simplemente quitar la referencia a la rutina de manejo de interrupcion.*/
	if (index > MAX_EXCEPTIONS) {
		return;
	}
	exception_handlers[index] = NULL_INTERRUPT_HANDLER;

}

