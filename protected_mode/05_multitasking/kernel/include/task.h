/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Contiene las definiciones de las estructuras de datos y las
 * rutinas para gestionar tareas con diferentes niveles de privilegios.
 *
 */

#ifndef TASK_H_
#define TASK_H_

#include <pm.h>
#include <irq.h>
#include <physmem.h>
#include <generic_list.h>

/** @brief Tamaño predeterminado en bytes para la pila de la tarea.*/
#define TASK_STACK_SIZE KERNEL_STACK_SIZE

/** * @brief Tamaño predeterminado en bytes del heap de la tarea.  */
#define TASK_HEAP_SIZE (2 * KERNEL_STACK_SIZE)

/** @brief Tope de la pila del kernel. Área de memoria en la cual se configura
 * la pila del kernel. */
#define KERNEL_STACK_TOP 0x9FC00

/** @brief Estados de una tarea */
typedef enum task_state {
	/** @brief La tarea puede ser usada para ejecutar un programa */
	TASK_AVAILABLE = 0,
	/** @brief La tarea se encuentra en espera que se le asigne la CPU*/
	TASK_READY = 1,
	/** @brief La tarea tiene el control de la CPU */
	TASK_RUNNING = 2,
	/** @brief La tarea se encuentra bloqueada */
	TASK_BLOCKED = 3,
	/** @brief La tarea se encuentra finalizada y esperando a que se liberen
	 * los recursos asociados a ella. */
	TASK_FINISHED = 4
} task_state;

/** @brief Estructura de datos para las tareas. Los campos que referencian a la
 * memoria se crean como apuntadores, para facilitar las operaciones
 * de memoria.
 */
typedef struct task {

	/** @brief Identificador de la tarea */
	unsigned short pid;

	/** @brief Identificador de la tarea padre */
	unsigned short ppid;

	/** @brief Estado actual de la tarea */
	unsigned char state;

	/** @brief Nivel de privilegio de la tarea */
	unsigned char pl;

	/** @brief Desplazamiento de la rutina inicial de la tarea */
	unsigned int entry;

	/** @brief Selector de codigo para la tarea. */
	unsigned short code_selector;

	/** @brief Selector de datos para la tarea. */
	unsigned short data_selector;

    /** @brief Tope de la pila en el espacio del kernel. */
    unsigned int kernel_stack;

    /** @brief Valor del apuntador al tope de la pila en el cual se
     * encuentra el marco actual de interrupcion. */
	unsigned int current_esp;

	/** @brief Numero de ticks de reloj que la tarea ha tenido la CPU. */
	unsigned int timeslice;

	/** @brief Total de ticks asignados a la tarea. */
	unsigned int total_ticks;

	/** @brief Definición de los apuntadores a la lista de tareas disponibles.
	 * Una tarea solo se puede encontrar en una lista de tareas en estado
	 * de disponible. */
	DEFINE_GENERIC_LIST_LINKS(available_task);

	/** @brief Definición de los apuntadores a la lista de tareas en estado de
	 * listo. Una tarea solo se puede encontrar en una lista de tareas
	 * en estado de listo. */
	DEFINE_GENERIC_LIST_LINKS(ready_task);
}task;

/** @brief Definición de las primitivas para gestionar listas de tipo
 * available_task */
DEFINE_GENERIC_LIST_TYPE(task, available_task);

/** @brief Definición de las primitivas para gestionar listas de tipo
 * ready_task */
DEFINE_GENERIC_LIST_TYPE(task, ready_task);

/** @brief Variable global que apunta a la tarea actual. */
extern task * current_task;

/** @brief Referencia para una tarea nula */
#define NULL_TASK 0

/*
 * @brief Número de la interrupcion asociado a la llamada al sistema. Este valor
 * debe estar sincronizado con la constante del mismo nombre en el archivo
 * usr/include/stdlib.h  */
#define INT_SYSCALL 0x80

/*
 * @brief Número de ticks de reloj que se asignan a una tarea en forma
 * continua. Luego de este número de ticks, la CPU se asigna a otra tarea.
 * Las tareas de kernel tienen un quantum de 50 ticks de reloj */
#define KERNEL_TASK_QUANTUM 50
/*
 * @brief Número de ticks de reloj que se asignan a una tarea en forma
 * continua. Luego de este número de ticks, la CPU se asigna a otra tarea.
 * Las tareas de usuario tienen un quantum de 20 ticks de reloj */
#define USER_TASK_QUANTUM 20

/** @brief Bandera global que indica si el entorno multitarea esta listo */
extern char multitasking_ready;

/*
 * @brief Variable global del kernel que almacena el selector del segmento
 * de pila que se encontraba activo cuando ocurrió una interrupción
 * */
extern unsigned short current_ss;

/** @brief Variable global del kernel que almacena el apuntador a la
 * pila en la posición en la cual se encuentra el estado del procesador
 * cuando ocurre una interrupción */
extern unsigned int current_esp;


/* funcion auxiliar para una tarea inicializada en 0 */
static __inline__ task null_task() {
	return (task) {0};
}

/** @brief Número maximo de tareas que pueden existir en un momento dado */
#define TASK_LIMIT 1024

/**
 * @brief Inicializa las estructuras de datos necesarias para
 * que el kernel pueda administrar múltiples tareas.
 */
void setup_multitasking(void);

/**
 * @brief Inicia la ejecucion de las tareas creadas en la inicializacion
 * del kernel.
 */
void start_multitasking(void);

/** @brief Compara una tarea con otra, para determinar si son iguales. */
int compare_task(task * a, task * b);

/** @brief Compara una tarea con un escalar. */
int equals_task(task * a, void * b);

/*
 * @brief Crea una nueva tarea (unidad de ejecución).
 * @param addr Direccion de la rutina inicial de la tarea (idle_task)
 * */
task * create_task(unsigned int start);

/**
 * @brief Permite determinar si dos tareas se encuentran en el mismo nivel de
 * privilegios.
 * @param task1 Tarea 1 a comparar
 * @param task2 Tarea 2 a comparar
 * @return 1 = las tareas tienen el mismo PL, 0 = las tareas tienen diferente PL
 */
static __inline__ int same_pl(task * task1, task * task2) {

	return ((task1 != NULL_TASK && task2 != NULL_TASK) &&
			(task1->pl == task2->pl));
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
void switch_to_task(task * t);

/**
 * @brief Esta rutina busca la siguiente tarea que se debe ejecutar
 * y le entrega el control. Si no existe una tarea disponible,
 * le retorna el control a la rutina 'idle_task' como una tarea.
 */
void task_dispatcher(void);

/**
 * @brief Esta rutina marca la tarea como terminada, y luego invoca
 * al despachador de tareas.
 * @param t Tarea a finalizar */
void finish_task(task * t);

/**
 * @brief Retorna el numero de tareas creadas.
 * @return Numero de tareas creadas.
 * */
int get_task_count();

/**
 * @brief Esta rutina retorna un apuntador a la tarea actual.
 * @return Apuntador a la tarea actual.
 * */
task * get_current_task();

#endif /* TASK_H_ */
