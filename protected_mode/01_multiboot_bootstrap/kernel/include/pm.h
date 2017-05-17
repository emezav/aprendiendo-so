/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 * @brief Contiene las definiciones relacionadas con el Modo Protegido IA-32.
 */

#ifndef PM_H_
#define PM_H_

/** @brief Dirección física del kernel en memoria */
#define KERNEL_PHYS_ADDR 0x100000

/* @brief Tamaño de la pila del kernel */ 
#define KERNEL_STACK_SIZE 0x4000

#endif
