/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Este archivo implementa las rutinas para gestionar tareas con
 * diferentes niveles de privilegios.
 * @details Para ofrecer un entorno de ejecución a las tareas se debe crear
 * dos nuevos descriptores de segmento en la GDT para describir los segmentos
 * de código y datos de las tareas con PL=3.
 *
 * Igualmente se debe crear un TSS y un descriptor de segmento de TSS que
 * también se almacena en la GDT.
 *
 * Las entradas en la GDT para las tareas se configuran cada vez
 * que ocurre un cambio de contexto. Esto permite aislar las tareas para
 * prevenir  que tengan acceso al espacio de otras tareas o del kernel.
 */

#include <asm.h>
#include <console.h>
#include <elf.h>
#include <generic_list.h>
#include <irq.h>
#include <kmem.h>
#include <paging.h>
#include <physmem.h>
#include <pm.h>
#include <stdlib.h>
#include <string.h>
#include <task.h>
#include <tss.h>
#include <timer.h>

/** @brief Tabla de tareas del kernel.
 * @details
 * En esta estructura de datos se almacenan las tareas que son gestionadas
 * por el kernel. */
task tasks[TASK_LIMIT];

/** @brief Lista de tareas en estado TASK_AVAILABLE.
 * @details Las referencias de esta lista apuntan a las entradas de la tabla
 * tasks del kernel que se encuentran disponibles para asignar a una
 * nueva tarea */
list_available_task available_tasks;

/** @brief Lista de tareas en estado TASK_READY.
 * @details Las referencias de esta lista apuntan a las entradas de la tabla
 * tasks del kernel que se encuentran marcadas con estado TASK_READY */
list_ready_task ready_tasks;

/** @brief Variable local que referencia a la tarea actual (la tarea en
 * ejecución) */
task * current_task = 0;

/** @brief Selector que referencia al descriptor de segmento de código de la
 * tarea, almacenado en la GDT. */
unsigned short task_code_selector;

/** @brief Selector que referencia al descriptor de segmento de datos de la
 * tarea, almacenado en la GDT. */
unsigned short task_data_selector;

/** @brief Variable global del kernel que almacena el apuntador al tope
 * de la pila del kernel. */
unsigned int kernel_esp;

/** @brief Bandera global que indica si el entorno multitarea esta listo */
char multitasking_ready = 0;

/** @brief Numero de tareas creadas en el espacio del kernel. */
int task_count = 0;

/** @brief Callback para recibir notificaciones del timer. */
void task_timer_callback(unsigned int);

/**
 * @brief A partir de un marco de interrupcion, continua con la ejecucion
 * de una tarea. Implementada en isr.S
 */
extern void return_from_interrupt(void);

/**
 * @brief Esta rutina recibe el control del administrador de tareas,
 * cuando no existe una tarea definida.
 * @details
 * Esta tarea se ejecuta en el contexto del kernel, con las interrupciones
 * habilitadas. Su proposito consiste en  detener el procesador, hasta que
 * ocurra una nueva interrupcion.
 */
void idle_task(void);

/**
 * @brief Recibe el control cuando una tarea de kernel "retorna".
 * @details
 * Esta tarea se ejecuta en el contexto del kernel, con las interrupciones
 * habilitadas. Su proposito consiste en recibir el control de las tareas
 * de kernel. Debe finalizar la tarea actual.
 */
void task_sink(void);

/**
 * @brief Esta rutina permite obtener la dirección de la posicion
 * del apuntador de la pila (esp) de la tarea actual.
 * @return Apuntador al tope de pila la tarea en el momento en que fue
 * interrumpida.
 */
unsigned int  get_current_task_esp(void);

/**
 * @brief Busca una entrada libre en la tabla de tareas del kernel.
 * Se considera una entrada libre aquella cuyo atributo state  sea igual
 * a TASK_AVAILABLE
 * @return Apuntador a una entrada disponible en la tabla de tareas del kernel.
 */
static __inline__ task * allocate_task();

/**
 * @brief Establece la variable del kernel current_task a la
 * tarea especificada.
 * @param t Apuntador a la tarea que se convierte en la tarea actual.
 */
void set_current_task(task * t);

/**
 * @brief Imprime el marco de interrupcion de una tarea,
 * apuntado por su atributo current_esp
 * @param t Apuntador a la tarea
 */
void print_task_context(task *);

/**
 * @brief Establece la variable del kernel current_task a la
 * tarea especificada.
 * @param t Apuntador a la tarea que se convierte en la tarea actual.
 */
void set_current_task(task * t) {
	current_task = t;
}

/**
 * @brief Esta rutina retorna un apuntador a la tarea actual.
 * @return Apuntador a la tarea actual.
 * */
task * get_current_task() {
	return current_task;
}

/**
 * @brief Esta rutina permite manejar la interrupción invocada por
 * una tarea. Se instala como manejador de la interrupción INT_SYSCALL.
 * @param state Apuntador al marco de interrupción de la tarea actual
 */
void task_handler(interrupt_state * state) {

	unsigned int esp;
	unsigned int * stack;
	unsigned int i;

	/* Si no existe una tarea actual, retornar. */
	if (current_task == NULL_TASK) {return;}

	/* Recuperar el apuntador actual a la pila de la tarea. */

	console_printf("Task handler called! current esp: %x\n", current_esp);

	esp = get_current_task_esp();

	/* Simplemente finalizar la tarea que invoco la llamada al sistema. */
	finish_task(current_task);

	/* Invocar al despachador de tareas*/
	task_dispatcher();

}

/**
 * @brief Permite insertar valores en la pila de una tarea.
 * @param t Tarea a la cual se insertara el valor en su pila.
 * @param value  Valor a insertar en la pila de la tarea
 * @note Esta rutina supone que se ha configurado una pila
 * válida para la tarea, cuyo tope se encuentra en (t->current_esp).
 */
static __inline__ push_into_task_stack(task *t, unsigned int value) {
	if (t == NULL_TASK) {
		console_printf("Warning! pushing a value into a null task\n");
		return;
	}

	if (t->current_esp == 0) {
		console_printf("Warning! pushing a value into stack with top=0\n");
		return;
	}

	/* Decrementar el tope de la pila de la tarea */
	t->current_esp -= sizeof(unsigned int);

	/* Decrementar el tope de la pila de la tarea */
	*((unsigned int*)(t->current_esp))=  value;
}

/** @brief Referencia a la Rutina de Servicio de Interrupción (ISR) encargada
 * de manejar las interrupción INT_SYSCALL */
extern void isr128(void);

/**
 * @brief Inicializa las estructuras de datos necesarias para
 * que el kernel pueda administrar múltiples tareas.
 */
void setup_multitasking(void) {
	int i;

	/* Referencia a la IDT, para modificarla y permitir el acceso a una
	 * interrupcion por software de las tares con cualquier PL*/
	extern idt_descriptor  idt[];

	/* Referencia al selector de codigo del kernel */
	extern unsigned short kernel_code_selector;

	/* Valores para almacenar el numero de entrada de la GDT para los
	 * segmentos de codigo y datos de las tareas*/
	int code_index;
	int data_index;

    /* Configurar el Segmento de Estado de Tarea (TSS) y el Registro de
     * Tareas (Task Register)*/
    setup_tss();

	/* Variable para almacenar el indice dentro del GDT que se asignara al
	 * descriptor de TSS para las tareas*/
	//unsigned short tss_index;

	/** - Inicializar las listas de tareas disponibles y en estado de listo */
	init_list_available_task(&available_tasks);
	init_list_ready_task(&ready_tasks);

	/** - Inicializar todas las tareas a nulo. Esto las marca con estado
	 * TASK_AVAILABLE */

	for (i = 0; i < TASK_LIMIT; i++) {
		tasks[i] = null_task();
		tasks[i].pid = i;
		push_back_available_task(&available_tasks, &tasks[i]);
	}

	/** - Antes de crear la primera tarea de kernel, se deben configurar dos
	 * nuevos descriptores dentro de la GDT.
	 *
	 * - Primero se crea un descriptor de segmento de código vacío.
	 * - Luego se crea un descriptor de datos vacío.
	 * Tanto el descriptor de segmento de código como el descriptor de segmento
	 * de datos se configuran adecuadamente cada vez que el kernel
	 * le pase el control a una tarea. */

	task_code_selector = allocate_gdt_selector();

	if (!task_code_selector) {
		console_printf("Unable to allocate a GDT entry for the user code segment!\n");
		return;
	}

	/** - allocate_gdt_entry() solo marca los descriptores como 'Presente'. Sus
	 * demas bits se establecen a 0. Esto es valido por ahora, ya que
	 * la entrada realmente se configura cada vez que se va a realizar
	 * un cambio de contexto. No obstante se debe reservar la entrada en
	 * la GDT y configurarla como vacia.*/

	code_index = task_code_selector >> 3;

    /* console_printf("Task code selector: %x\n", task_code_selector);*/

	/** - Reservar y establecer en nulo la entrada en la GDT para el
	 * descriptor de segmento de datos para la tarea */

	task_data_selector = allocate_gdt_selector();

	if (!task_data_selector) {
		console_printf("Unable to allocate a GDT entry for the user data segment!\n");
		return;
	}

	data_index = task_data_selector >> 3;

	/*console_printf("Task data selector: %x\n", task_data_selector);*/

	set_current_task(NULL_TASK);

	/** - Instalar el manejador de interrupción para permitir que las
	 * tares soliciten servicios al kernel. En este caso se instala
	 * un manejador para la interrupcion 128 (0x80).*/
	install_interrupt_handler(INT_SYSCALL, task_handler);

	/** - Configurar la entrada 128 de la IDT para que sea una interrupt
	 * gate accesible desde diferentes niveles de privilegio.
	 * Al asignar RING3_DPL, esta interrupt gate puede ser invocada desde
	 * los niveles de privilegio 0 (mayor privilegio) a 3 (menor privilegio).
	 * En otras palabras, cualquier tarea puede invocar la interrupción
	 * por software 0x80 (128 en decimal). */

    setup_idt_descriptor(INT_SYSCALL, kernel_code_selector, 
            (unsigned int)isr128,
            RING3_DPL, INTERRUPT_GATE_TYPE);

    /** - Adicionar un callback de timer. */
    add_timer_callback(task_timer_callback);

}

/**
 * @brief Inicia la ejecucion de las tareas creadas en la inicializacion
 * del kernel.
 */
void start_multitasking(void) {
    /** - Entorno multi - tarea listo. */
    multitasking_ready = 1;

    /** - Pasar el control a la primera tarea en la cola de listos. */
    task_dispatcher();
}

/** @brief Callback para recibir notificaciones del timer. */
void task_timer_callback(unsigned int ticks) {
    /* Si no se ha configurado el entorno multitarea, retornar. */
    if (!multitasking_ready) {
        return;
    }

	/* Si existe una tarea en ejecucion, aumentar su uso de CPU. */
	if (current_task != NULL_TASK) {
		current_task->timeslice++;
	}
}

/**
 * @brief Busca una entrada libre en la tabla de tareas del kernel.
 * Se considera una entrada libre aquella cuyo atributo state  sea igual
 * a TASK_AVAILABLE
 * @return Apuntador a una entrada disponible en la tabla de tareas del kernel.
 */
static __inline__ task * allocate_task() {
	task * ret;

	/* Obtener una entrada disponible dentro de la tabla de tareas */

	ret = pop_front_available_task(&available_tasks);

	if (ret != 0) {
		if (current_task != NULL_TASK) {
			ret->ppid = current_task->pid;
		} else {
			ret->ppid = -1;
		}
	}else {
		console_printf("Warning! could not allocate task entry!\n");
		ret = NULL_TASK;
	}

	return ret;
}

/*
 * @brief Crea una nueva tarea (unidad de ejecución).
 * @param addr Direccion de la rutina inicial de la tarea (idle_task)
 * */
task * create_task(unsigned int start) {

	unsigned short code_selector;
	unsigned short data_selector;
    unsigned int kernel_stack_start;

	int i;

	task * ret;

    if (start == 0) {
        start = (unsigned int)idle_task;
    }


	/** - Buscar una entrada disponible dentro de la tabla de tareas
	 * del kernel */
	ret = allocate_task();

	//console_printf("Allocated task: %d\n", ret->pid);

	/**    - Si no existe una entrada disponible, terminar! */
	if (ret == NULL_TASK) {
		console_printf("Warning! no task available found!\n");
		return ret;
	}


	/** - Reservar memoria para la pila de la tarea */
	kernel_stack_start = kmem_allocate_pages(TASK_STACK_SIZE / PAGE_SIZE, 0);

    if (kernel_stack_start == 0) {
        console_printf("Could not allocate %d pages for the new task!\n",
                TASK_STACK_SIZE / PAGE_SIZE
                );
        return NULL_TASK;
    }

    /* El tope de la pila se encuentra al final de las paginas reservadas.*/
    ret->kernel_stack = (unsigned int)kernel_stack_start + TASK_STACK_SIZE;
    
	/** - Inicio de la tarea */
	ret->entry = start;

	//console_printf("Task entry at %x (%d)\n", ret->ret->entry);

	/** - El nivel de privilegios de la tarea */
	ret->pl = RING0_DPL;

	/** - Definir los selectores de código y datos para la tarea */
	code_selector = (task_code_selector >> 3) << 3 | ret->pl;
	data_selector = (task_data_selector >> 3) << 3 | ret->pl;

	/* console_printf("Code selector: %x Data selector: %x\n", code_selector,
	 data_selector); */

    ret->code_selector = code_selector;
    ret->data_selector = data_selector;

	ret->current_esp = ret->kernel_stack;

	/**    - Reservar el espacio para el manejo de las interrupciones */
	ret->current_esp -= (sizeof(interrupt_state) / sizeof(unsigned int)) + 1;

	/**  - La pila de la tarea ahora luce asi:
	 * @verbatim
	 +--------------------------+ <- ret->current_esp
	 |                          |
	 |                          |  Espacio reservado para que cada vez
	 |  espacio vacio en el     |  que ocurra una interrupcion, el kernel
	 |  tope de la pila de la   |  automaticamente apunte ss:esp al tope de
	 |  tarea:                  |  la pila de la tarea si PL != 0.
	 |  Reservado para el kernel|
	 |  (tareas con PL !=0)     |
	 |  Para tareas con PL = 0, |  Para tareas con PL = 0, no ocurre un
	 |  este espacio no se usa. |  cambio de pila. No obstante se reserva el
	 |                          |  espacio para mantener un marco de pila
	 |                          |  estandar.
	 |--------------------------|<-- ret->current_esp
	 @endverbatim
	 */

	/**  - Guardar la direccion de la rutina task_sink, si la tarea
     * "retorna". */
    push_into_task_stack(ret, (unsigned int) task_sink);

	/**  - La pila de la tarea ahora luce asi:
	 * @verbatim
	 +--------------------------+ <- ret->kernel_stack
	 |                          |
	 |                          |  Espacio reservado para que cada vez
	 |  espacio vacio en el     |  que ocurra una interrupcion, el kernel
	 |  tope de la pila de la   |  automaticamente apunte ss:esp al tope de
	 |  tarea:                  |  la pila de la tarea si PL != 0.
	 |  Reservado para el kernel|
	 |  (tareas con PL !=0)     |
	 |  Para tareas con PL = 0, |  Para tareas con PL = 0, no ocurre un
	 |  este espacio no se usa. |  cambio de pila. No obstante se reserva el
	 |                          |  espacio para mantener un marco de pila
	 |                          |  estandar.
	 |--------------------------|
	 | &task_sink               |
	 |--------------------------| t->current_esp
	 @endverbatim
	 */

	/**    - Configurar el contexto inicial de la tarea. El contexto se
	 * configura para que parezca que la tarea ha sido interrumpida y su estado
	 * se ha almacenado en su pila. */

	/**    - Si el PL de la tarea es != 0, almacenar el apuntador actual al tope
	 * de la pila. De esta forma, cuando se retorne de la interrupcion,
     * la tarea retornará a su pila original.
	 * Para tareas con PL = 0, estos valores no se insertan en la pila.
	 *  */
	if (ret->pl != RING0_DPL) {
			push_into_task_stack(ret, ret->data_selector); /* old ss */
			push_into_task_stack(ret, ret->current_esp
										+ sizeof (unsigned int)); /* old esp */
			/* Se le debe sumar sizeof(unsigned int) debido a que se
			 * incluyo current_ss en la pila ese valor debe ser sacado
			 * automaticamente al momento de retornar de la interrupcion */
	}

	/**    - Almacenar en la pila de la tarea un estado inicial, que le permita
	 * al kernel "Retornar de interrupción" en el espacio de la tarea.*/
	push_into_task_stack(ret, IF_ENABLE); /* old_eflags */
	push_into_task_stack(ret, ret->code_selector); /* old cs */
	push_into_task_stack(ret, ret->entry); /* old eip  = entry point */
	push_into_task_stack(ret, 0); /* codigo de error */
	/* Valores almacenados en el mismo orden que las rutinas de
	 * servicio de interrupcion */
	push_into_task_stack(ret, 0); /* interrupcion generada */
	push_into_task_stack(ret, 0); /* eax *//* pusha*/
	push_into_task_stack(ret, 0); /* ecx */
	push_into_task_stack(ret, 0); /* edx */
	push_into_task_stack(ret, 0); /* ebx */
	push_into_task_stack(ret, 0); /* esp antes de pusha, ignorado*/
	push_into_task_stack(ret, 0); /* ebp */
	push_into_task_stack(ret, 0); /* esi */
	push_into_task_stack(ret, 0); /* edi */

	push_into_task_stack(ret, ret->data_selector); /* ds */
	push_into_task_stack(ret, ret->data_selector); /* es */
	push_into_task_stack(ret, ret->data_selector); /* fs */
	push_into_task_stack(ret, ret->data_selector); /* gs */

	/**    - Finalmente la pila para la nueva tarea se encuentra asi:
     * @verbatim
	 +--------------------------+ <- ret->kernel_stack
	 |                          |
	 |                          |  Espacio reservado para que cada vez
	 |  espacio vacio en el     |  que ocurra una interrupcion, el kernel
	 |  tope de la pila de la   |  automaticamente apunte ss:esp al tope de
	 |  tarea:                  |  la pila de la tarea.
	 |  Reservado para el kernel|
	 |  (tareas con PL !=0)     |
	 |  Para tareas con PL = 0, |  Para tareas con PL = 0, no ocurre un
	 |  este espacio no se usa. |  cambio de pila. No obstante se reserva el
	 |                          |  espacio para mantener un marco de pila
	 |                          |  estandar.
	 |--------------------------|
	 | &task_sink               |
	 |--------------------------| <-+
	 | old ss                   |   | (Estos valores solo se almacenan para
	 |--------------------------|   |  tareas con PL != 0)
	 | old esp                  | --+
	 |--------------------------|
	 | eflags                   |   IF = 1
	 |--------------------------|
	 | old cs                   | Selector de codigo de la tarea
	 |--------------------------|
	 | old eip                  | Función inicial de la tarea (main)
	 |--------------------------|
	 | 0 (codigo de error)      | 0
	 |--------------------------|
	 | # de excepcion generada  | 0
	 |--------------------------|
	 | eax                      | 0
	 |--------------------------|
	 | ecx                      | 0
	 |--------------------------|
	 | edx                      | 0
	 |--------------------------|
	 | ebx                      | 0
	 |--------------------------|
	 | esp antes de pusha       | (ignorado)
	 |--------------------------|
	 | ebp                      | 0
	 |--------------------------|
	 | esi                      | 0
	 |--------------------------|
	 | edi                      | 0
	 |--------------------------|------------------------------------------
	 | ds                       | Selector de datos de la tarea
	 |--------------------------|
	 | es                       | Selector de datos de la tarea
	 |--------------------------|
	 | fs                       | Selector de datos de la tarea
	 |--------------------------|
	 | gs                       | Selector de datos de la tarea
	 |--------------------------|<-- ret->current_esp
	 @endverbatim
	 */

	/** - Se debe cambiar el estado de la tarea e insertarla en la lista
	 * de tareas en estado "listo" */

	ret->state = TASK_READY;
    push_back_ready_task(&ready_tasks, ret);

    /** - Incrementar el numero de tareas creadas. */
    task_count++;

	return ret;
}


/**
 * @brief Crea una nueva tarea, a partir de un archivo ELF que se
 * encuentra leido en memoria.
 * @param command  Argumentos que se deben pasar a la tarea
 * @param start Dirección de inicio del archivo ELF en memoria
 * @param end Dirección de fin del archivo ELF en memoria
 * @param pl Nivel de privilegio de la tarea
 * */
task * load_program(task * t, char * command) {

	char * task_location;
	unsigned int size; /* Tamanio del codigo y los datos de la tarea */
	unsigned int limit; /* Tamanio de la tarea, incluyendo pila y heap */
	unsigned int entry; /* Desplazamiento de la función main() del programa */
	unsigned int data_offset; /* Desplazamiento de la seccion de datos de la
	 tarea */

    unsigned int start;
    unsigned int end;
    char pl;

	unsigned short code_selector;
	unsigned short data_selector;

	int i;

	/* Variables utilizadas para copiar los argumentos que se pasaran como
	 * parametro a la rutina main() de la tarea.
	 * De antemano se supone que maximo se pueden pasar 255 parametros
	 * a la tarea. */
	int argc;
	char * argv[255];

	int command_offset;
	int argument_offset;
	int char_count;
	char token[255];

	task * ret;

	/* Si ocurre cualquier error, retornar una tarea nula. */
	ret = NULL_TASK;

	/** -  Tratar de leer el encabezado del modulo: Se supone que cada módulo
	 * es un archivo en formato ELF. Si no lo es, retornar.
	 * Leer la documentacion del formato ELF para mas detalles.*/

	/** - Los archivos con formato ELF comienzan con un encabezado, en el cual
	 * se encuentra informacióon descriptiva.*/
	elf_header * header = (elf_header *) start;

	if (header == 0) {
		console_printf("Warning! module has no valid ELF header\n");
		return ret;
	}

	/**   - Si es un archivo ELF válido, los campos ident[0..3]
	 * deben contener 0x7f, 'E', 'L', 'F', respectivamente. */
	if (header->ident[0] != 0x7f || header->ident[1] != 'E' || header->ident[2]
			!= 'L' || header->ident[3] != 'F') {
		console_printf("Warning! module has no valid ELF header\n");
		return ret;
	}

	/** - Buscar una entrada disponible dentro de la tabla de tareas
	 * del kernel */
	ret = allocate_task();

	//console_printf("Allocated task: %d\n", ret->pid);

	/**    - Si no existe una entrada disponible, terminar! */
	if (ret == NULL_TASK) {
		console_printf("Warning! no task available found!\n");
		return ret;
	}
	//console_printf("Program: %s Command: %s\n", program, command_line);

	entry = header->entry;

	/**   - Si el encabezado ELF es valido, obtener el mapa de las secciones
	 * de programa: */
	program_header * ph;
	//console_printf(" ELF at %u\n", start);
	int count;
	size = 0;
	for (ph = (program_header*) (start + header->program_header_offset), count
			= 0; count < header->program_header_count; ph++, count++) {
		/**      - Buscar los segmentos validos, para determinar  el tamaño del
		 * código y los datos de la tarea en memoria. */
		if (ph->type == ELF_PT_LOAD && ph->virtual_address <= end - start
				&& ph->file_size <= ph->memory_size) {
			/*
			 console_printf("Program header at %u\n", ph);
			 console_printf("align: %d filesize: %u flags: %b memory_size: %u "
			 "offset: %u phys: %u virt: %u, type=%d\n", ph->align,
			 ph->file_size, ph->flags, ph->memory_size, ph->offset,
			 ph->physical_address, ph->virtual_address, ph->type);
			 */

			if (ph->align == 0 || ph->align == 1) {
				size = size + ph->memory_size;
			} else {
				size = size + ph->align;
			}
		}
	}

	/** - Redondear el tamaño de la tarea a un limite de unidades de memoria */
	size = ROUND_UP_TO_PAGE(size);

	/** - Agregar el espacio requerido para el heap y la pila de la tarea */
	limit = size + TASK_HEAP_SIZE + TASK_STACK_SIZE;

	//console_printf("Executable size: %u Task size: %u\n", size, limit);

	/** - Reservar memoria para la tarea */
	task_location = (char* ) kmem_allocate_pages((limit / PAGE_SIZE) + 1, 1);
	memset(task_location, 0, limit);

	/*
	console_printf("task location: %x -> %x \n", task_location, task_location + limit);
	*/

	/* - Copiar las secciones desde el archivo ELF leido en memoria
	 * al espacio de la tarea. Para ello se debe recorrer de nuevo los
	 * encabezados de programa (program_header) */
	for (ph = (program_header*) (start + header->program_header_offset), count
			= 0; count < header->program_header_count; ph++, count++) {
		/**    - Sólo copiar los segmentos de programa que sean válidos:
		 * de tipo ELF_PT_LOAD y cuya dirección virtual se encuentre
		 * dentro del limite del archivo ejecutable.
		 * Igualmente, de acuerdo con la especificacion ELF, el atributo
		 * file_size no debe ser mayor que el atributo memory_size
		 * */
		if (ph->type == ELF_PT_LOAD && ph->virtual_address <= end - start
				&& ph->file_size <= ph->memory_size) {
			/*
			 console_printf("Program header at %u\n", ph);
			 console_printf("align: %d filesize: %u flags: %b memory_size: %u "
			 "offset: %u phys: %u virt: %u, type=%d\n", ph->align,
			 ph->file_size, ph->flags, ph->memory_size, ph->offset,
			 ph->physical_address, ph->virtual_address, ph->type);
			 */

			/* Copiar a la posicion de la tarea*/
			/*
			 console_printf("Copying to %x from %u %d bytes\n",
			 (char*)((unsigned int)task_location +
			 ph->virtual_address),
			 (char *)(start + ph->offset), ph->file_size);
			 */
			//console_printf("flags: %b type: %b ", ph->flags, ph->type);
			/**    - Si el program header referencia la seccion de datos
			 * del ejecutable, almacenar su desplazamiento
			 * desde el inicio.*/
			if (ph->type == ELF_PT_LOAD && (ph->flags & (ELF_PH_WRITE))
					&& (ph->flags & (ELF_PH_READ))) {
				data_offset = ph->virtual_address;
				//console_printf("data offset at: %u\n", data_offset);
			}

			memcpy(
					(char*) (task_location
							+ ph->virtual_address), (char *) (start
							+ ph->offset), ph->file_size);
		}
	}

	/** - Inicializar la sección de argumentos de la tarea en 0.
	 * Esta sección se ubica siempre al inicio de su seccion de datos,
	 * y tiene un tamaño predefinido de 512 bytes. Ver archivo start.S,
	 * en su seccion .bootdata*/

	memset((void *) (task_location + data_offset), 0, 512);

	/** - Copiar los argumentos que se pasaron a la tarea en su seccion
	 * de argumentos. */

	/** - Siempre la tarea recibe al menos un argumento, el nombre de
	 * programa. Este siempre es el primer argumento.*/
	argc = 0;
	command_offset = 0;
	argument_offset = 0;
	char_count = 0;

	/*console_printf("Copying arguments to: %x\n", task_location + data_offset); */

	/*console_printf("Command: %s\n", command);*/
	do {
		/* La rutina nexttok automaticamente retorna una cadena terminada
		 * en nulo. */
		strcpy(token, "");
		char_count = nexttok(command, token, 0, command_offset);
		/*console_printf("count: %d token: %s", char_count, token);*/
		if (char_count > 0) {
			strcpy(task_location + data_offset + argument_offset, token);
			argv[argc] = (char*) (data_offset + argument_offset);
			command_offset = command_offset + char_count;
			/* Aumentar argument_offset en la longitud de la
			 * cadena + 1 (Caracter nulo al final de la misma)*/
			argument_offset = argument_offset + char_count + 1;
			/*console_printf("Argument %x: %s\n", task_location +
										(unsigned int)argv[argc],
										task_location +
										(unsigned int)argv[argc]);*/
			argc++;
		}
	} while (char_count > 0);
	//console_printf("\n");

	if (argc == 0) {
		console_printf("No argument copied!\n");
	}

	/** - Desplazamiento a partir de la dirección base en el cual
	 *  se encuentra la rutina a la cual se pasa el control inicialmente. */
	ret->entry = entry;

	//console_printf("Task entry at %x (%d)\n", ret->base + ret->entry, ret->entry);

	/** - El nivel de privilegios de la tarea */

	ret->pl = pl;

	/** - Definir los selectores de código y datos para la tarea */
	code_selector = (task_code_selector >> 3) << 3 | ret->pl;
	data_selector = (task_data_selector >> 3) << 3 | ret->pl;
	/* console_printf("Code selector: %x Data selector: %x\n", code_selector,
	 data_selector); */

	ret->data_selector = data_selector;
	ret->current_esp = 0; //TODO ajustar la pila para la tarea!!!


	/**    - Reservar el tope de la pila para uso del kernel.
	 * Para tareas con PL != 0: El TSS se configuro de forma tal
	 * que cada vez que ocurre una interrupcion, el kernel apunta
	 * automaticamente al tope de la pila de la tarea interrumpida. Debido
	 * a que el kernel debe almacenar el marco de interrupcion, se debe
	 * reservar el espacio equivalente al tamaño de la estructura
	 * interrupt_state */
	/**    - Reservar el espacio para el manejo de las interrupciones */
	ret->current_esp -= sizeof(interrupt_state);

	/**    - La pila de la tarea ahora luce asi:
	 * @verbatim
	 +--------------------------+ <- ret->base + ret->limit (tope de la pila)
	 |                          |
	 |                          |  Espacio reservado para que cada vez
	 |  espacio vacio en el     |  que ocurra una interrupcion, el kernel
	 |  tope de la pila de la   |  automaticamente apunte ss:esp al tope de
	 |  tarea:                  |  la pila de la tarea.
	 |  Reservado para el kernel|
	 |  (tareas con PL !=0)     |
	 |  Para tareas con PL = 0, |  Para tareas con PL = 0, no ocurre un
	 |  este espacio no se usa. |  cambio de pila. No obstante se reserva el
	 |                          |  espacio para mantener un marco de pila
	 |                          |  estandar.
	 |--------------------------|<-- ret->current_esp
	 @endverbatim
	 */

	/**    - Copiar los argumentos que se pasaran a la rutina principal de la
	 * tarea (main) en su pila. Estos se almacenan de atras hacia adelante, es
	 * decir primero el argumento n-1, luego el argumento n-2 ... hasta el
	 * argumento 0. */
	for (i = 1; i <= argc; i++) {
		push_into_task_stack(ret, (unsigned int) argv[argc - i]);
	}

	/**    - Almacenar la dirección de argv en la pila */
	push_into_task_stack(ret, ret->current_esp);

	/**    - Almacenar el número de argumentos (argc) */
	push_into_task_stack(ret, argc);

	/**    - La pila de la tarea ahora luce asi:
	 * @verbatim
	 +--------------------------+ <- ret->base + ret->limit (tope de la pila)
	 |                          |
	 |                          |  Espacio reservado para que cada vez
	 |  espacio vacio en el     |  que ocurra una interrupcion, el kernel
	 |  tope de la pila de la   |  automaticamente apunte ss:esp al tope de
	 |  tarea:                  |  la pila de la tarea.
	 |  Reservado para el kernel|
	 |  (tareas con PL !=0)     |
	 |  Para tareas con PL = 0, |  Para tareas con PL = 0, no ocurre un
	 |  este espacio no se usa. |  cambio de pila. No obstante se reserva el
	 |                          |  espacio para mantener un marco de pila
	 |                          |  estandar.
	 |--------------------------|
	 | ...                      |
	 |--------------------------| Argumentos a la rutina 'main' de la tarea
	 |argv[1]                   |
	 |--------------------------|
	 |argv[0]                   |
	 |--------------------------|
	 |argv                      |
	 |--------------------------|
	 |argc (# de argumentos)    |
	 |--------------------------|
	 @endverbatim
	 */

	/**    - Configurar el contexto inicial de la tarea. El contexto se
	 * configura para que parezca que la tarea ha sido interrumpida y su estado
	 * se ha almacenado en su pila. */


	/**    - Si el PL de la tarea es != 0, almacenar el apuntador actual al tope
	 * de la pila. De esta forma, cuando se retorne de la interrupcion,
	 * en el tope de la pila se encuentran los argumentos de la rutina 'main'.
	 * Para tareas con PL = 0, estos valores no se insertan en la pila.
	 *  */
	if (ret->pl != RING0_DPL) {
			push_into_task_stack(ret, ret->data_selector); /* old ss */
			push_into_task_stack(ret, ret->current_esp
										+ sizeof (unsigned int)); /* old esp */
			/* Se le debe sumar sizeof(unsigned int) debido a que se
			 * incluyo current_ss en la pila ese valor debe ser sacado
			 * automaticamente al momento de retornar de la interrupcion */
	}

	/**    - Almacenar en la pila de la tarea un estado inicial, que le permita
	 * al kernel "Retornar de interrupción" en el espacio de la tarea.*/
	push_into_task_stack(ret, IF_ENABLE); /* old_eflags */
	push_into_task_stack(ret, code_selector); /* old cs */
	push_into_task_stack(ret, entry); /* old eip  = entry point del ejecutable*/
	push_into_task_stack(ret, 0); /* codigo de error */
	/* Valores almacenados en el mismo orden que las rutinas de
	 * servicio de interrupcion */
	push_into_task_stack(ret, 0); /* interrupcion generada */
	push_into_task_stack(ret, 0); /* eax *//* pusha*/
	push_into_task_stack(ret, 0); /* ecx */
	push_into_task_stack(ret, 0); /* edx */
	push_into_task_stack(ret, 0); /* ebx */
	push_into_task_stack(ret, 0); /* esp antes de pusha, ignorado*/
	push_into_task_stack(ret, 0); /* ebp */
	push_into_task_stack(ret, 0); /* esi */
	push_into_task_stack(ret, 0); /* edi */

	push_into_task_stack(ret, data_selector); /* ds */
	push_into_task_stack(ret, data_selector); /* es */
	push_into_task_stack(ret, data_selector); /* fs */
	push_into_task_stack(ret, data_selector); /* gs */

	/**    - Finalmente la pila para la nueva tarea se encuentra asi:
	 * @verbatim
	 +--------------------------+ <- ret->base + ret->limit (tope de la pila)
	 |                          |
	 |                          |  Espacio reservado para que cada vez
	 |  espacio vacio en el     |  que ocurra una interrupcion, el kernel
	 |  tope de la pila de la   |  automaticamente apunte ss:esp al tope de
	 |  tarea:                  |  la pila de la tarea.
	 |  Reservado para el kernel|
	 |  (tareas con PL !=0)     |
	 |  Para tareas con PL = 0, |  Para tareas con PL = 0, no ocurre un
	 |  este espacio no se usa. |  cambio de pila. No obstante se reserva el
	 |                          |  espacio.
	 |--------------------------|
	 | ...                      |
	 |--------------------------| Argumentos a la rutina 'main' de la tarea
	 |argv[1]                   |
	 |--------------------------|
	 |argv[0]                   |
	 |--------------------------|
	 |argc (# de argumentos)    |
	 |--------------------------| <-+
	 | old ss                   |   | (Estos valores solo se almacenan para
	 |--------------------------|   |  tareas con PL != 0)
	 | old esp                  | --+
	 |--------------------------|
	 | eflags                   |   IF = 1
	 |--------------------------|
	 | old cs                   | Selector de codigo de la tarea
	 |--------------------------|
	 | old eip                  | Función inicial de la tarea (main)
	 |--------------------------|
	 | 0 (codigo de error)      | 0
	 |--------------------------|
	 | # de excepcion generada  | 0
	 |--------------------------|
	 | eax                      | 0
	 |--------------------------|
	 | ecx                      | 0
	 |--------------------------|
	 | edx                      | 0
	 |--------------------------|
	 | ebx                      | 0
	 |--------------------------|
	 | esp antes de pusha       | (ignorado)
	 |--------------------------|
	 | ebp                      | 0
	 |--------------------------|
	 | esi                      | 0
	 |--------------------------|
	 | edi                      | 0
	 |--------------------------|------------------------------------------
	 | ds                       | Selector de datos de la tarea
	 |--------------------------|
	 | es                       | Selector de datos de la tarea
	 |--------------------------|
	 | fs                       | Selector de datos de la tarea
	 |--------------------------|
	 | gs                       | Selector de datos de la tarea
	 |--------------------------|<-- ret->current_esp
	 @endverbatim
	 */

	/** - Se debe cambiar el estado de la tarea e insertarla en la lista
	 * de tareas en estado "listo" */

	ret->state = TASK_READY;
    push_back_ready_task(&ready_tasks, ret);

	return ret;
}

/**
 * @brief Descripcion: Busca la siguiente tarea disponible de la lista de
 * tareas.
 */
task * find_next_ready_task() {
	return pop_front_ready_task(&ready_tasks);
}

/**
 * @brief Esta rutina determina el quantum asignado a la tarea actual.
 * @return quantum de la tarea actual, dependiendo de su nivel de privilegios.
 *
 */
int current_quantum() {
	if (current_task == NULL_TASK) {
		return USER_TASK_QUANTUM;
	}
	if (current_task->pl == RING0_DPL) {
		return KERNEL_TASK_QUANTUM;
	}else {
		return USER_TASK_QUANTUM;
	}
}

/**
 * @brief Esta rutina busca la siguiente tarea que se debe ejecutar
 * y le entrega el control. Si no existe una tarea disponible,
 * le retorna el control a la función idle_task() como una tarea.
 */
void task_dispatcher(void) {
	int i;
	task * new_task;


	/** - Si la tarea actual ha finalizado, informar. */
	if (current_task != NULL_TASK && current_task->state == TASK_FINISHED) {
		finish_task(current_task);
	}

	/** - Si existe una tarea actual y su estado es 'en ejecucion',
	 * retornar a esa tarea siempre y cuando su quantum no haya terminado. */
	if (current_task != NULL_TASK && current_task->state == TASK_RUNNING) {
        /*
        console_printf("switch to running task. Stack: 0x%x -> 0x%x\n",
                current_task->kernel_stack, current_task->kernel_stack - TASK_STACK_SIZE);
        */
		/** - Si la tarea actual aun tiene quantum, retornar a ella.*/
		if (current_task->timeslice < current_quantum()) {
			/**       - Establecer el estado de la tarea a 'listo', para poder
			 * invocar la rutina switch_to_task */
			current_task->state = TASK_READY;
			switch_to_task(current_task);
			/** - En caso contrario, buscar una nueva tarea.*/
		}else {
			/** -  Buscar la siguiente tarea a ejecutar */
			new_task = find_next_ready_task();

			/** - Antes de pasarle el control a la nueva tarea, actualizar
			 * su estado y timeslice */
			current_task->state = TASK_READY;
			current_task->total_ticks += current_task->timeslice;
			current_task->timeslice = 0;

			/** - Si existe otra tarea, pasarle el control. */
			if (new_task != NULL_TASK) {
			/** - Insertar la tarea que sale de la CPU en la cola de listos */
				push_back_ready_task(&ready_tasks, current_task);
				/* Pasar el control a la nueva tarea*/
				new_task->timeslice = 0;
				switch_to_task(new_task);
			}else {
			/** -  Si no existe otra tarea, retornar el control a la tarea
		    * actual */
				switch_to_task(current_task);
			}
		}
	}

	/** - Si no existe una tarea actual, buscar una nueva tarea para pasarle
	 *  el control */
	/**      - Buscar una tarea en estado de listo para ejecutar */
	new_task = find_next_ready_task();
	/** - Si existe otra tarea, pasarle el control. */
	if (new_task != NULL_TASK) {
		/* Pasar el control a la nueva tarea*/
		new_task->timeslice = 0;
		switch_to_task(new_task);
	}

    /* Mover el apuntador de la pila al espacio para el kernel*/
    inline_assembly("movl %0, %%esp" : : "a"(kernel_stack_end));

    /* Crear la idle task */
	new_task = create_task((unsigned int)idle_task);
    if (new_task == NULL_TASK) {
        console_printf("Could not create idle task!\n");
        for(;;);
    }
    switch_to_task(new_task);
}

/**
 * @brief Permite continuar la ejecucion de una tarea especificada.
 * Realiza los ajustes para cambiar de nivel de privilegio si es necesario.
 * @param t Tarea a convertirse en la tarea actual.
 * @note Esta rutina no retorna, ya que la ejecucion continua en la
 * tarea especificada.
 * @note Esta rutina asume que los atributos current_ss y
 * current_esp de la tarea apuntan al inicio de un marco de interrupcion valido.
 */
void switch_to_task(task * t) {

	int i;

	extern tss kernel_tss;
	extern unsigned short kernel_tss_selector;
	extern unsigned short kernel_code_selector;
	extern unsigned short kernel_data_selector;
    extern unsigned int kernel_pd_addr;

	/** - Si la tarea no existe o no se encuentra en estado de listo, retornar.
	 */
	if (t == NULL_TASK || t->state != TASK_READY) {
		return;
	}

	/** - Establecer el estado de la tarea como Ejecutando */
	t->state = TASK_RUNNING;

	/** - Configurar el descriptor de segmento de código de tarea para
	 * que apunte al espacio de la tarea  */
	setup_gdt_descriptor(t->code_selector,
					0,
					0xFFFFFFFF,
					CODE_SEGMENT,
					t->pl,
					1,
					1);

	/** - Configurar el descriptor de segmento de datos de tarea para
	 * que apunte al espacio de la tarea  */
	setup_gdt_descriptor(t->data_selector,
					0,
					0xFFFFFFFF,
					DATA_SEGMENT,
					t->pl,
					1,
					1);

	/** - Re-iniciar el registro de estado de tareas (TSS) del kernel. */
	kernel_tss = NULL_TSS;

    kernel_tss.cr3 = kernel_pd_addr;

	/** -  Configurar el apuntador de la pila, para que cuando suceda una
	 * interrupcion se configure automaticamente ss y esp con estos valores.
	 * Cuando una tarea con PL !=0 es interrumpida, el procesador toma
	 * los valores de ss y esp (el apuntador a la pila) de los atributos
	 * ss0 y esp0 del TSS actual.*/

	/**    - ss cuando ocurre una interrupcion: selector de datos del
     * kernel  */
	kernel_tss.ss0 = kernel_data_selector;

	/**    -  esp cuando ocurre una interrupcion: Tope de la pila de la tarea.
	 * Para facilitar el manejo de la pila desde el manejador de interrupcion,
	 * se toma una dirección lineal absoluta en el segmento de pila del kernel.
	 */
	kernel_tss.esp0 = (unsigned int)t->kernel_stack;

	/** - Configurar de nuevo el selector de TSS del kernel, para que
	 * apunte a la tarea actual con el nivel de privilegio requerido. */
	setup_gdt_descriptor(kernel_tss_selector,
			(unsigned int)&kernel_tss,
			sizeof(tss) - 1,
			TASK_TYPE,
			t->pl,
			0,
			0);

    //console_printf("Load task register\n");

	/** Cargar el Task Register con el valor del selector configurado */
	inline_assembly("ltr %0" : : "a"(kernel_tss_selector) );

	/** -  Establecer la variable global 'current_task' a la tarea especificada
	*/
	set_current_task(t);

	/** - Apuntar al marco de interrupcion de la tarea, para modificar
	 * el registro EFLAGS.
	 * En el momento en que ocurre la interrupcion, la isr correspondiente
	 * deshabilita las interrupciones y luego almacena el marco de
	 * interrupción.
	 * Se requiere que al retornar a la tarea, las interrupciones
	 * se encuentren habilitadas. Por esta razon se debe modificar
	 * el valor de old_eflags en el marco de interrupcion de la tarea. */
	interrupt_state * state = (interrupt_state *) t->current_esp;
    //console_printf("Interrupt enable\n");
	/** -  Activar las interrupciones en el registro EFLAGS para la tarea a la
	 * cual se va a pasar. */
	state->old_eflags |= IF_ENABLE;

	/** Retornar al marco de interrupción de la tarea.
	 * Para tareas con PL diferente de cero, se debe apuntar a una
	 * pila relativa al kernel.
	 * Para tareas con PL = 0, se apunta al marco de pila
	 * de la tarea.
	 * Las variables current_ss y current_esp se usan en la rutina
	 * return_from_interrupt() para apuntar al tope de la pila en la
	 * cual se encuentra el marco de interrupcion de la tarea a retornar el
	 * control.*/
	//if (t->pl != RING0_DPL) {
		current_ss = t->data_selector;
		current_esp = t->current_esp;
	//}else{
	//	current_ss = kernel_data_selector;
	//	current_esp = t->current_esp;
	//}
    //

	/** - La rutina return_from_interrupt hace uso de las variables current_ss
	 * y current_esp establecidas anteriormente, para apuntar ss:esp al
	 * marco de interrupcion de la tarea a la cual se debe retornar el
	 * control. */
	return_from_interrupt();

	/** - Esta rutina no retorna, ya que return_from_interrupt() le paso
	 * el control a la tarea especificada.*/
}

/**
 * @brief Imprime el marco de interrupcion de una tarea,
 * apuntado por su atributo current_esp
 * @param t Apuntador a la tarea
 */
void print_task_context(task *t) {

	interrupt_state * state;
	extern unsigned int current_esp;

	state = (interrupt_state *) (t->current_esp);

	console_printf("[%d] gs: %x fs: %x es: %x ds: %x\n", t->state, state->gs,
			state->fs, state->es, state->ds);

	console_printf("edi: %u esi: %u ebp: %u esp: %u ebx: %u edx: %u ecx: %u eax: %u\n",
			state->edi, state->esi, state->ebp, state->esp, state->ebx,
			state->ebx, state->edx, state->ecx, state->eax);
	console_printf("Number: %d Error code: %d old eip: %u old cs: %x\n", state->number,
			state->error_code, state->old_eip, state->old_cs);

	console_printf("Old eflags: %b old esp: %u old ss: %x\n", state->old_eflags,
			state->old_esp, state->old_ss);
}

/**
 * @brief Esta rutina recibe el control del administrador de tareas,
 * cuando no existe una tarea definida.
 * @details
 * Esta tarea se ejecuta en el contexto del kernel, con las interrupciones
 * habilitadas. Su proposito consiste en  detener el procesador, hasta que
 * ocurra una nueva interrupcion.
 */
void idle_task(void) {
	while (1) {
		/* Detener el procesador hasta que ocurra otro tick de reloj. */
		inline_assembly("hlt");
	}
}

/**
 * @brief Recibe el control cuando una tarea de kernel "retorna".
 * @details
 * Esta tarea se ejecuta en el contexto del kernel, con las interrupciones
 * habilitadas. Su proposito consiste en recibir el control de las tareas
 * de kernel. Debe finalizar la tarea actual.
 */
void task_sink(void) {
    
    extern unsigned int tmp_stack;

    inline_assembly("movl %0, %%esp" : : "a"(tmp_stack));

    if (current_task != NULL_TASK) {
        finish_task(current_task);
    }else {
        set_current_task(NULL_TASK);
    }
    task_dispatcher();
}


/**
 * @brief Esta rutina marca la tarea como terminada, y luego invoca
 * al despachador de tareas.
 * @param t Tarea a finalizar */
void finish_task(task * t){
    unsigned int addr;
    unsigned int sz;

	if (t == NULL_TASK) {return;}

	t->total_ticks += t->timeslice;
	/*console_printf("Task %d finished! total ticks: %u total secs: %d\n",
			t->pid,
			t->total_ticks,
			t->total_ticks / TIMER_FREQUENCY);*/

	/** Realizar la limpieza de la tarea. */

    /** - Marcar la tarea como finalizada */
    t->state = TASK_FINISHED;

    
    /*
    console_printf("Free stack 0x%x -> 0x%x\n", 
            t->kernel_stack,
            t->kernel_stack - TASK_STACK_SIZE);
    */
    
    

    /** - Liberar la pila asignada a la tarea */
    for (addr = t->kernel_stack - TASK_STACK_SIZE, sz = TASK_STACK_SIZE;
            addr < t->kernel_stack;
            addr += PAGE_SIZE, sz -= PAGE_SIZE) {
        //console_printf("Free page 0x%x -> 0x%x\n", addr, addr + PAGE_SIZE  );
        kmem_free(addr);
    }

    /** - Limpiar el área de memoria de la tarea*/
    memset(t, 0, sizeof(task));

    /** - Marcar la entrada como disponible */
    t->state = TASK_AVAILABLE;

    /** - Almacenar la referencia a la tarea en la lista de tareas
     * disponibles */
    push_back_available_task(&available_tasks, t);

	/* La tarea a finalizar es la actual? */
	if (t == current_task) {
		set_current_task(NULL_TASK);
	}

    /** - Decrementar el numero de tareas creadas. */
    task_count--;

}

/**
 * @brief Esta rutina permite obtener la dirección de la posicion
 * del apuntador de la pila (esp) de la tarea actual.
 * @return Apuntador al tope de pila la tarea en el momento en que fue
 * interrumpida.
 */
unsigned int get_current_task_esp(void) {

	unsigned int ret;
	unsigned int ptr;
	interrupt_state * state;

	/** - Validar si la tarea actual esta definida */
	if (current_task == NULL_TASK){
		return 0;
	}

	/**
	 * - Para tareas con PL = RING0_DPL, el marco de interrupcion se almacenó
	 * en la posición actual de la pila
	 * Para tareas con PL != RING0_DPL, el marco de interrupcion se almacenó
	 * en el tope de la pila.
	 */

	/** Obtener la dirección lineal de la posicion actual de la pila */
	ptr = (unsigned int)current_task->current_esp;

	/** - Hallar el apuntador a la pila que la tarea tenia justo antes de que
	 * ocurriera la interrupcion */
	if (current_task->pl == RING0_DPL) {

	/** - Para tareas con PL = RING0_DPL, el marco de interrupcion se
	 * almaceno en el tope actual de la pila. De esta forma se debe
	 * sumar a ptr el tamaño en bytes de  un marco de
	 * interrupcion, y restarle el tamaño de old_ss y old_esp ya que
	 * a las tareas de tipo 0 estos valores no se almacenan
	 * en el marco de interrupcion. */

	/** actual + marco de interrupcion - (tamaño (old_ss) + tamaño(old_esp))*/
		ptr += current_task->current_esp + 
                sizeof(interrupt_state)
				- (sizeof(unsigned int) * 2);
	}else {
		/**
		 * Para tareas con PL != RING0_DPL, existen dos alternativas:
		 * a) el marco de interrupcion siempre se almacena en el limite de
		 * la pila:
		 * @verbatim
		 +----------------------+ <- current_task->kernel_stack
		 | old ss               |
		 +----------------------+
		 | old esp              | <- Valor de esp a obtener
		 +----------------------+
		 | ....                 | (resto del marco de interrupcion)
		 +----------------------+
		 @endverbatim
		 b) current_task->current_esp apunta al marco de interrupción.
		 De este marco de interrupcion se puede obtener el valor de old_esp.
		 */

		/* Obtener un apuntador al marco de interrupcion actual. */
		interrupt_state * state = (interrupt_state *)(
				(unsigned int)current_task->current_esp
		);

		/* A partir del marco de interrupcion, obtener el atributo old_esp
		 * old_esp es relativo a la tarea, por lo cual se debe sumar
		 * a current_task->base para obtener un apuntador lineal.
		 * */
		ptr += state->old_esp;
	}

	/* El primer valor encontrado en la pila es la dirección a la cual se debe
	 * retornar en el espacio de la tarea para continuar la ejecucion.
	 * Avanzar en la pila, para apuntar a los parametros que se encuentran
	 * en la pila de la tarea. */
	ret = ptr + sizeof(unsigned int);

	return ret;
}

/**
 * @brief Retorna el numero de tareas creadas.
 * @return Numero de tareas creadas.
 * */
int get_task_count() {
    return task_count;
}

