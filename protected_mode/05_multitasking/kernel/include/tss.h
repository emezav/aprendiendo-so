/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Contiene las definiciones de las estructuras de datos y las
 * rutinas asociadas con la gestion de segmentos de estado de tarea.
 */

#ifndef TSS_H_
#define TSS_H_

/** @brief Task State Segment (TSS). Esta estructura de datos representa el
 * estado de una tarea en la arquitectura IA-32. Consulte el manual de
 * Intel IA-32 Volume 3 Systems Programming para más detalles.
 * @details
 * El TSS es una estructura de datos usada por IA-32 para implementar la
 * multi-tarea.
 * Debido a que se usará multi-tarea por software, los únicos campos
 * requeridos son ss0 y esp0, que representan la posición de la pila
 * para el nivel de privilegios 0. En esta posición se almacena el estado
 * del procesador cuando se interrumpe la ejecución de la tarea.
 */
typedef struct tss {
	/** @brief Enlae a la Tarea anterior  */
	unsigned short link;
	/** @brief Espacio reservado luego de link  para completar 32 bits*/
	unsigned short padding_link;
	/** @brief ESP para el nivel de privilegio 0: apunta al desplazamiento en
	 *  la pila en el cual se debe almacenar el estado del procesador cuando
	 *  se interrumpe la tarea */
	unsigned int esp0;
	/** @brief SS para el nivel de privilegio 0. Selector que referencia al
	 *  descriptor de segmento de datos dentro de la GDT para la pila.*/
	unsigned short ss0;
	/** @brief Espacio reservado luego de ss0 para completar 32 bits*/
	unsigned short padding_ss0;
	/** @brief ESP para el nivel de privilegio 1 */
	unsigned int esp1;
	/** @brief SS para el nivel de privilegio 1 */
	unsigned short ss1;
	/** @brief Espacio reservado luego de ss1 para completar 32 bits */
	unsigned short padding_ss1;
	/** @brief ESP para el nivel de privilegio 2 */
	unsigned int esp2;
	/** @brief SS para el nivel de privilegio 2 */
	unsigned short ss2;
	/** @brief Espacio reservado luego de ss2 para completar 32 bits */
	unsigned short padding_ss2;
	/** @brief Valor de CR3  */
	unsigned int cr3;
	/** @brief valor de EIP */
	unsigned int eip;
	/** @brief Valor de EFLAGS */
	unsigned int eflags;
	/** @brief Valor de EAX */
	unsigned int eax;
	/** @brief Valor de ECX */
	unsigned int ecx;
	/** @brief Valor de EDX */
	unsigned int edx;
	/** @brief Valor de EBX */
	unsigned int ebx;
	/** @brief Valor de ESP */
	unsigned int esp;
	/** @brief Valor de EBP */
	unsigned int ebp;
	/** @brief Valor de ESI */
	unsigned int esi;
	/** @brief Valor de EDI */
	unsigned int edi;
	/** @brief Valor de ES */
	unsigned short es;
	/** @brief Espacio reservado luego de ES para completar 32 bits */
	unsigned short es_padding;
	/** @brief Valor de CS */
	unsigned short cs;
	/** @brief Espacio reservado luego de CS para completar 32 bits */
	unsigned short cs_padding;
	/** @brief Valor de SS */
	unsigned short ss;
	/** @brief Espacio reservado luego de SS para completar 32 bits */
	unsigned short ss_padding;
	/** @brief Valor de DS */
	unsigned short ds;
	/** @brief Espacio reservado luego de DS para completar 32 bits */
	unsigned short ds_padding;
	/** @brief Valor de FS */
	unsigned short fs;
	/** @brief Espacio reservado luego de FS para completar 32 bits */
	unsigned short fs_padding;
	/** @brief Valor de GS */
	unsigned short gs;
	/** @brief Espacio reservado luego de GS para completar 32 bits */
	unsigned short gs_padding;
	/** @brief Selector de la LDT para la tarea */
	unsigned short ldt_selector;
	/** @brief Espacio reservado para completar 32 bits */
	unsigned short ldt_selector_padding;
	/** @brief Espacio antes de iomap_base para completar 32 bits */
	unsigned short t;
	/** @brief Dirección base de la Base del Mapa de E/S*/
	unsigned short iomap_base;

}tss;

/** @brief TSS con todos sus campos en nulo.*/
#define NULL_TSS  \
	(tss){ \
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 \
}

/** @brief Constante que define un tipo de descriptor de TSS (BUSY = 0)*/
#define TASK_TYPE 0x09

/** @brief Constante que define un tipo de descriptor de TSS (BUSY = 0)*/
#define BUSY_TASK_TYPE 0x0B

/** @brief Definición del segmento de estado de tarea (TSS) para el kernel. */
extern tss kernel_tss;

/**
 * @brief Esta función establece el registro de estado de tarea (TSS) para
 * el kernel.
 */
void setup_tss(void);

#endif /* TSS_H_ */
