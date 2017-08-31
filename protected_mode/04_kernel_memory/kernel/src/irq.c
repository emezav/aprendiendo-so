/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Contiene la implementacion de las rutinas necesarias para
 * manejar las solicitudes de interrupcion (IRQ) de los dispositivos de
 * entrada / salida.
 */

#include <pm.h>
#include <asm.h>
#include <console.h>
#include <irq.h>
#include <stdlib.h>

/** @brief Arreglo que contiene los apuntadores a las rutinas de manejo de
 * interrupción
 */
irq_handler irq_handlers[MAX_IRQ_ROUTINES];

/**
 * @brief Recibe el control de la rutina de manejo de
 * interrupcion y re-envia esta solicitud a la rutina de manejo de IRQ
 * correspondiente, si se encuentra definida.
 * Dentro de la estructura de datos que recibe, se puede obtener el numero
 * de la interrupcion que ocurrio, asi como el estado del procesador.
 * Con el numero de la interrupcion y restando IDT_IRQ_OFFSET se puede
 * obtener el numero de IRQ.
 * @param state Apuntador al estado del procesador cuando ocurre la IRQ
 */
void irq_dispatcher(interrupt_state * state);

/**
 * @brief Función que se encarga de re-mapear las IRQ 0x8 a 0xF.
 * @details Al arranque, las IRQ 0 a 7 estan mapeadas a las interrupciones
 * 0x8 - 0xF. Estas entradas estan asignadas por defecto a las excepciones x86
 * (la entrada 8 es Double Fault, por ejemplo).
 * Por esta razon es necesario mapear las IRQ a otros numeros de interrupcion.
 */
void irq_remap(void);


/* Implementación de las rutinas */

void irq_remap(void) {
	/** Reprogramación del PIC  */

	/** Initialization Command Word 1 - ICW1
	Esta es la palabra primaria para inicializar el PIC.
	Para inicializar el PIC se requiere que los bits 0 y 4 de ICW1 esten en
	1 y los demas en 0. Esto significa que el valor de ICW1 es 0x11.
	ICW1 debe ser escrita en el  registro de comandos del PIC maestro
	(dirección de e/s 0x20). Si existe un PIC esclavo, ICW1 se debe enviar
	tambien su registro de comandos del PIC esclavo (0xA0)
	*/

	outb(MASTER_PIC_COMMAND_PORT, 0x11);
	outb(SLAVE_PIC_COMMAND_PORT, 0x11);

	/** Initialization Command Word 2 - ICW2
	Esta palabra permite definir la dirección base (inicial) en la tabla de
	descriptores de interrupcion que el PIC va a utilizar.

	Debido a que las primeras 32 entradas estan reservadas para las
	excepciones en la arquitectura IA-32, ICW2 debe contener un valor mayor o
	igual a 32 (0x20). Los valores  de ICW2 representan el numero de IRQ
	base que manejara el PIC

	Al utilizar los PIC en cascada, se debe enviar ICW2 a los dos
	controladores en su registro de datos (0x21 y 0xA1 para maestro y
	esclavo respectivamente), indicando la dirección en la IDT que va a ser
	utilizada por cada uno de ellos.

	Las primeras 8 IRQ van a ser manejadas por el PIC maestro y se mapearan
	a partir del numero 32 (0x20). Las siguientes 8 interrupciones las manejara
	el PIC esclavo, y se mapearan a partir de la interrupcion 40 (0x28).
	*/

	outb(MASTER_PIC_DATA_PORT, IDT_IRQ_OFFSET);
	outb(SLAVE_PIC_DATA_PORT, IDT_IRQ_OFFSET + 8);

	/** Initialization Control Word 3 - ICW3
	Esta palabra permite definir cuales lineas de IRQ van a ser compartidas
	por los PIC maestro y esclavo. Al igual que ICW2, ICW3 tambien se
	escribe en los 	registros de datos de los PIC (0x21 y 0xA1 para el PIC
	maestro y esclavo,respectivamente).

	Dado que en la arquitectura Intel el PIC maestro se conecta con el PIC
	esclavo por medio de la linea IRQ 2, el valor de ICW3 debe ser 00000100
	(0x04), que define el bit 3 (correspondiente a la linea IRQ2) en 1.

	Para el PIC esclavo, el numero de la linea se debe representar en
	notacion binaria. Por lo tanto, 000 corresponde a la linea de IRQ 0,
	001 corresponde a la linea de IRQ 1, 010 corresponde a la linea de
	IRQ 2, y asi sucesivamente.
	Debido a que se va a utilizar la linea de IRQ 2, el valor de ICW3
	para el PIC esclavo debe ser 00000010, (0x02).
	*/

	outb(MASTER_PIC_DATA_PORT, 0x04);
	outb(SLAVE_PIC_DATA_PORT, 0x02);

	/** Initialization Control Word 4 - ICW4
	Para ICW4 solo es necesario establecer su bit 0 (x86 mode) y escribirla
	en los registros de datos del PIC maestro y esclavo (0x21 y 0xA1).
	El valor de ICW4 debe ser entonces 00000001, es decir, 0x01.
	*/
	outb(MASTER_PIC_DATA_PORT, 0x01);
	outb(SLAVE_PIC_DATA_PORT, 0x01);

	/** Se han mapeado las IRQ!.
	 * Las IRQ 0-7 seran atendidas por el PIC maestro, y las IRQ 8-15
	 * por el PIC esclavo. Las IRQ0-15 estaran mapeadas en la IDT a partir
	 * de la entrada 32 hasta la 47.*/
}

/**
 * @brief Crea los manejadores de interrupcion para las 16 IRQ en los
 * procesadores x86. Estas IRQ  se re-mapean a las interrupciones con
 * vector 32 .. 47.
 * Todas ellas son manejadas por la rutina 'irq_dispacther', que se encarga
 * de invocar la rutina de manejo de IRQ correspondiente, si se encuentra
 * definida.
 */
void setup_irq(void) {
	int i;

	for (i=0; i<MAX_IRQ_ROUTINES;i++) {
		irq_handlers[i] = NULL_INTERRUPT_HANDLER;
	}

	/* Mapear las IRQ 0..15 a las entradas 32 .. 47 de la IDT */
		irq_remap();

	/* Ahora configurar el manejador para las interrupciones re-mapeadas
	 * (32..47)
	 * Todas estas interrupciones son manejadas por la rutina irq_dispatcher */

	for (i=0; i<MAX_IRQ_ROUTINES; i++) {
		install_interrupt_handler(i + IDT_IRQ_OFFSET, irq_dispatcher);
	}
}

/**
 * @brief Permite definir un nuevo manejador de IRQ
 * @param number numero de irq a configurar
 * @param handler Función a manejar la irq
 */
void install_irq_handler(int number, irq_handler handler){
	/* Simplemente sobre-escribir la rutina anterior, si existe.
	 * */
	if (number > MAX_IRQ_ROUTINES) {
		return;
	}
	irq_handlers[number] = handler;
}

/**
 * @brief Permite quitar un  manejador de IRQ.
 *
 * 	@param number numero de irq a quitar
 * 	@return void*/
void uninstall_irq_handler(int number) {

	if (number < MAX_IRQ_ROUTINES) {
		irq_handlers[number] = NULL_INTERRUPT_HANDLER;
	}
}

/**
 * @brief Recibe el control de la rutina de manejo de
 * interrupcion y re-envia esta solicitud a la rutina de manejo de IRQ
 * correspondiente, si se encuentra definida.
 * Dentro de la estructura de datos que recibe, se puede obtener el numero
 * de la interrupcion que ocurrio, asi como el estado del procesador.
 * Con el numero de la interrupcion y restando IDT_IRQ_OFFSET se puede
 * obtener el numero de IRQ.
 * @param state Apuntador al estado del procesador cuando ocurre la IRQ
 */
void irq_dispatcher(interrupt_state * state) {

	irq_handler handler;

	int index;

	/* Determinar el numero de la IRQ */
	index = state->number - IDT_IRQ_OFFSET;

	/*
	 * Enviar EOI al puerto de control del 8259 que lanzo la interrupcion
	 * */

	/* Si la IRQ es 8..15, se debe enviar EOI al 8259 esclavo tambien. */
	if (index >= 8) {
		outb(SLAVE_PIC_COMMAND_PORT, EOI);
	}
  
	outb(MASTER_PIC_COMMAND_PORT, EOI);

	/* Buscar la rutina que maneja la interrupcion */
	handler = irq_handlers[index];

	/* Si la rutina existe, ejecutarla y pasarle como parametro los
	 * registros.*/
	if (handler != NULL_INTERRUPT_HANDLER) {
			handler(state);
	}else {
		/* En caso contrario ignorar la interrupcion. */
        /*
		console_printf(" Warning! unhandled IRQ %d (INT %d)", index, state->number);
        for (;;);
        */
	}
}
