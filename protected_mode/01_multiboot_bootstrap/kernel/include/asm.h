/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 *
 * @brief Rutinas para la ejecución de código ensamblador para la arquitectura
 * IA-32

Dado que algunas instrucciones sólo se pueden ejecutar directamente en lenguaje
ensamblador, se hace uso de inline assembly (assembler en-linea) para
incluir código en ensamblador directamente dentro del código en C.

@verbatim

__asm__ __volatile__(" instrucciones_asm"
					 : operandos de salida
					 : operandos de entrada
					 : registros a invalidar
					 );
@endverbatim

@see http://www.ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html
HOWTO de assembler en línea.
 */

#ifndef ASM_H_
#define ASM_H_

/**
 * @brief Alias para incluir codigo ensamblador dentro del código en C
 *
 */
#define inline_assembly(code...) \
		__asm__ __volatile__(code)

/* Punto de depuración mágico de Bochs. Debe estar habilitado en el archivo de
 * configuración bochsrc*/
#define bochs_break() \
    inline_assembly("xchg %bx, %bx")

/**
 * @brief Lee un byte de un puerto de entrada / salida.
 * @param port Puerto de E/S del cual se debe leer el byte
 * @return Byte leido del puerto.
 */
static __inline__ unsigned char inb(unsigned short port) {
	unsigned char data;
	inline_assembly("inb %1,%0" : "=a" (data) : "dN" (port));
	return data;
}

/**
 * @brief Escribe un byte en un puerto de entrada / salida.
 *  @param port Puerto de E/S al cual se puede escribir el byte
 *  @param data Byte a escribir en el puerto.
 *
 */
static __inline__ void outb(unsigned short port, unsigned char data){
	inline_assembly("outb %1,%0" : : "dN" (port), "a" (data));
}

/**
 * @brief Lee un word de un puerto de entrada / salida.
 * @param port Puerto de E/S del cual se debe leer el byte
 * @return Word leido del puerto.
 */
static __inline__ unsigned short inw(unsigned short port) {
	unsigned short data;
	inline_assembly("inw %1,%0" : "=a" (data) : "dN" (port));
	return data;
}

/**
 * @brief Escribe un word en un puerto de entrada / salida.
 * @param port  Puerto de E/S al cual se puede escribir el word
 * @param data Word a escribir en el puerto.
 * @return void
 */
static __inline__ void outw(unsigned short port, unsigned short data){
	inline_assembly("outw %1,%0" : : "dN" (port), "a" (data));
}

#endif /* ASM_H_ */
