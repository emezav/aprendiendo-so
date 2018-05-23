/**
 * @file
 * @ingroup kernel_code
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License.
 *
 * @brief Este archivo implementa las rutinas para la gestión de listas de
 * tareas.
 */

#include <task.h>

/** @brief Compara una tarea con otra, para determinar si son iguales. */
int compare_task(task * a, task * b) {
	if (a == 0) {return (unsigned int)b;}
	if (b == 0) {return (unsigned int)a;}

	return b->pid - a->pid;
}

/** @brief Compara una tarea con un escalar. Compara por el PID de la tarea.*/
int equals_task(task * a, void * b) {
	if (a == 0) {return (unsigned int)b;}
	if (b == 0) {return (unsigned int)a;}
	return *((int*)b) - a->pid;
}

/** @brief Implementación de las primitivas para gestionar listas de tipo
 * available_task */
IMPLEMENT_GENERIC_LIST_TYPE(task, available_task);

/** @brief Implementación de las primitivas para gestionar listas de tipo
 * ready_task */
IMPLEMENT_GENERIC_LIST_TYPE(task, ready_task);



