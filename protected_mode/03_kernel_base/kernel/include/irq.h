/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Este archivo define las rutinas publicas para la gestion de
 * solicitudes de interrupcion (IRQ) de los dispositivos de Entrada / Salida
 */

#ifndef IRQ_H_
#define IRQ_H_

#include <pm.h>

/** @brief OCW2 (End of Interrupt): Codigo para escribir en el puerto de
 * comandos del PIC para indicar que se ha recibido la interrupción.
 */
#define EOI 0x20

/** @brief Dirección del puerto de comandos del PIC  maestro */
#define MASTER_PIC_COMMAND_PORT 0x20

/** @brief Dirección del puerto de comandos del PIC esclavo */
#define SLAVE_PIC_COMMAND_PORT 0xA0

/** @brief Dirección del puerto de datos del PIC maestro */
#define MASTER_PIC_DATA_PORT 0x21

/** @brief Dirección del puerto de datos del PIC esclavo */
#define SLAVE_PIC_DATA_PORT 0xA1

/** @brief Desplazamiento en la IDT a partir de la cual se configuran las
 * rutinas de manejo de interrupción. En IA-32, este debe ser mayor o igual a 32
 * debido a que las primeras 32 interrupciones son usadas por las
 * excepciones. */
#define IDT_IRQ_OFFSET 32

/** @brief Alias para el manejador de irq. */
typedef interrupt_handler irq_handler;

/* Constantes para los numeros de interrupcion de las IRQ0 - IRQ15.*/

/** @brief IRQ del Timer del Sistema. */
#define IRQ0_INTERRUPT IDT_IRQ_OFFSET + 0
/** @brief IRQ del Controlador de Teclado. */
#define IRQ1_INTERRUPT IDT_IRQ_OFFSET + 1
/** @brief Señal en cascada con el PIC esclavo. */
#define IRQ2_INTERRUPT IDT_IRQ_OFFSET + 2
/** @brief IRQ del Controlador del Puerto Serial COM2, si está presente.
 * Compartido con el puerto serial COM4. */
#define IRQ3_INTERRUPT IDT_IRQ_OFFSET + 3
/** @brief IRQ del Controlador del Puerto Serial COM1, si está presente.
 * Compartido con el puerto serial COM3. */
#define IRQ4_INTERRUPT IDT_IRQ_OFFSET + 4
/** @brief IRQ del puerto 2 de LPT o tarjeta de sonido */
#define IRQ5_INTERRUPT IDT_IRQ_OFFSET + 5
/** @brief IRQ Controlador de Disco Floppy */
#define IRQ6_INTERRUPT IDT_IRQ_OFFSET + 6
/** @brief IRQ del Controlador del puerto 1 de LPT o cualquier dispositivo
 * paralelo. */
#define IRQ7_INTERRUPT IDT_IRQ_OFFSET + 7
/** @brief IRQ del Timer RTC. */
#define IRQ8_INTERRUPT IDT_IRQ_OFFSET + 8
/** @brief IRQ disponible o usada por el controlador SCSI. */
#define IRQ9_INTERRUPT IDT_IRQ_OFFSET + 9
/** @brief IRQ disponible o usada por el controlador SCSI o NIC */
#define IRQ10_INTERRUPT IDT_IRQ_OFFSET + 10
/** @brief IRQ disponible o usada por el controlador SCSI o NIC */
#define IRQ11_INTERRUPT IDT_IRQ_OFFSET + 11
/** @brief IRQ del Mouse o Controlador PS/2  */
#define IRQ12_INTERRUPT IDT_IRQ_OFFSET + 12
/** @brief IRQ del Co-Procesador Matemático, FPU o interrupción inter-procesador.
 * a disposición del sistema  */
#define IRQ13_INTERRUPT IDT_IRQ_OFFSET + 13
/** @brief IRQ del Canal ATA primario. */
#define IRQ14_INTERRUPT IDT_IRQ_OFFSET + 14
/** @brief IRQ del Canal ATA SEcundario (para discos duros o CD)*/
#define IRQ15_INTERRUPT IDT_IRQ_OFFSET + 15

/** @brief Define el número máximo de rutinas de manejo de IRQ
 * que se pueden definir en el sistema.*/
#define MAX_IRQ_ROUTINES 16

/**
 * @brief Crear las entradas en la IDT para
 * las interrupciones que se desean manejar. Por defecto configura las
 * interrupciones correspondientes a las IRQ de los dispositivos de entradda/
 * salida, que han sido mapeadas a los numeros 32..47.
 */
void setup_irq(void);

/**
 * @brief Permite definir un nuevo manejador de IRQ
 * @param number numero de irq a configurar
 * @param handler Función a manejar la irq
 */
void install_irq_handler(int number, irq_handler handler);

/**
 * @brief Permite quitar un  manejador de IRQ.
 *
 * 	@param number numero de irq a quitar
 * 	@return void*/
void uninstall_irq_handler(int number);

#endif /* IRQ_H_ */
