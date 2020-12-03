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

__asm__ (" instrucciones_asm"
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
		__asm__ (code)

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
	inline_assembly("inb %w1, %b0" : "=a" (data) : "dN" (port));
	return data;
}

/**
 * @brief Escribe un byte en un puerto de entrada / salida.
 *  @param port Puerto de E/S al cual se puede escribir el byte
 *  @param data Byte a escribir en el puerto.
 *
 */
static __inline__ void outb(unsigned short port, unsigned char data){
	inline_assembly("outb %b1, %w0" : : "dN" (port), "a" (data));
}

/**
 * @brief Lee un word de un puerto de entrada / salida.
 * @param port Puerto de E/S del cual se debe leer el byte
 * @return Word leido del puerto.
 */
static __inline__ unsigned short inw(unsigned short port) {
	unsigned short data;
	inline_assembly("inw %w1, %w0" : "=a" (data) : "dN" (port));
	return data;
}

/**
 * @brief Escribe un word en un puerto de entrada / salida.
 * @param port  Puerto de E/S al cual se puede escribir el word
 * @param data Word a escribir en el puerto.
 * @return void
 */
static __inline__ void outw(unsigned short port, unsigned short data){
	inline_assembly("outw %w1, %w0" : : "dN" (port), "a" (data));
}

/**
 * @brief Lee un dword de un puerto de entrada / salida.
 * @param port Puerto de E/S del cual se debe leer el byte
 * @return dword leido del puerto.
 */
static __inline__ unsigned int inl(unsigned short port) {
	unsigned int data;
	inline_assembly("inl %w1, %0" : "=a" (data) : "dN" (port));
	return data;
}

/**
 * @brief Escribe un dword en un puerto de entrada / salida.
 * @param port  Puerto de E/S al cual se puede escribir el word
 * @param data Word a escribir en el puerto.
 * @return void
 */
static __inline__ void outl(unsigned short port, unsigned int data){
	inline_assembly("outl %1, %w0" : : "dN" (port), "a" (data));
}

/**
 * @brief Lee bytes desde un puerto de entrada/salida.
 * @param port Puerto de e/s del cual se lee los bytes
 * @param addr Buffer en donde se leen los bytes
 * @param count Cantidad de bytes a leer del puerto
 *
 * @return void
 */
static __inline__ void insb(unsigned short port, void * addr, unsigned int count) {
	inline_assembly("rep; insb": "+D"(addr), "+c"(count):"d"(port):"memory");
}

/**
 * @brief Lee words desde un puerto de entrada/salida.
 * @param port Puerto de e/s del cual se lee los words
 * @param addr Buffer en donde se leen los words
 * @param count Cantidad de words a leer del puerto
 *
 * @return void
 */
static __inline__ void insw(unsigned short port, void * addr, unsigned int count) {
	inline_assembly("rep; insw": "+D"(addr), "+c"(count):"d"(port):"memory");
}

/**
 * @brief Lee doublewords desde un puerto de entrada/salida.
 * @param port Puerto de e/s del cual se lee los doublewords
 * @param addr Buffer en donde se leen los doublewords
 * @param count Cantidad de doublewords a leer del puerto
 *
 * @return void
 */
static __inline__ void insl(unsigned short port, void * addr, unsigned int count) {
	inline_assembly("rep; insl": "+D"(addr), "+c"(count):"d"(port):"memory");
}

/**
 * @brief Escribe bytes a un puerto de entrada/salida.
 * @param port Puerto de e/s al cual se escriben los bytes
 * @param addr Buffer desde el cual se leen los bytes
 * @param count Cantidad de bytes a escribir en el puerto
 *
 * @return void
 */
static __inline__ void outsb(unsigned short port, void * addr, unsigned int count) {
	inline_assembly("rep; outsb" : : "S"(addr), "c"(count), "d"(port):"memory");
}

/**
 * @brief Escribe words a un puerto de entrada/salida.
 * @param port Puerto de e/s al cual se escriben los words
 * @param addr Buffer desde el cual se leen los words
 * @param count Cantidad de words a escribir en el puerto
 *
 * @return void
 */
static __inline__ void outsw(unsigned short port, void * addr, unsigned int count) {
	inline_assembly("rep; outsw" : : "S"(addr), "c"(count), "d"(port):"memory");
}

/**
 * @brief Escribe doublewords a un puerto de entrada/salida.
 * @param port Puerto de e/s al cual se escriben los doublewords
 * @param addr Buffer desde el cual se leen los doublewords
 * @param count Cantidad de doublewords a escribir en el puerto
 *
 * @return void
 */
static __inline__ void outsl(unsigned short port, void * addr, unsigned int count) {
	inline_assembly("rep; outsl" : : "S"(addr), "c"(count), "d"(port):"memory");
}



/**
 * @brief Intercambio atómico de un byte.
 * Compara el byte almacenado en AL (old) con el valor almacenado en
 * *lock. Si son iguales, se establece ZF y el valor new se almacena en
 * *lock. En caso contrario, limpia ZF y  almacena en AL el valor anterior
 * almacenado en *lock. El valor almaceando en AL se retorna de nuevo en
 * ret.
 @return old si se pudo realizar la operacion, new si no.
*/
static __inline__ unsigned char cmpxchgb(unsigned char * lock, 
        unsigned char old, 
        unsigned char new) {

    unsigned char ret;

    ret = old;

    inline_assembly("lock\n\t" \
                " cmpxchgb %2, %1" \
                : "=a" (ret), "+m" (*lock) \
                : "r" (new), "0" (old) \
                : "memory");
    return ret;
}

/**
 * @brief Intercambio atómico de un word.
 * Compara el word almacenado en AX (old) con el valor almacenado en
 * *lock. Si son iguales, se establece ZF y el valor new se almacena en
 * *lock. En caso contrario, limpia ZF y  almacena en AX el valor anterior
 * almacenado en *lock. El valor almaceando en AX se retorna de nuevo en
 * ret.
 @return old si se pudo realizar la operacion, new si no.
*/
static __inline__ unsigned short cmpxchgw(unsigned short * lock, 
                unsigned short old,
                unsigned short new) {

    unsigned short ret;

    ret = old;

    inline_assembly("lock\n\t" \
                " cmpxchgw %2, %1" \
                : "=a" (ret), "+m" (*lock) \
                : "r" (new), "0" (old) \
                : "memory");
    return ret;
}


/**
 * @brief Intercambio atómico de un dword (long).
 * Compara el word almacenado en EAX (old) con el valor almacenado en
 * *lock. Si son iguales, se establece ZF y el valor new se almacena en
 * *lock. En caso contrario, limpia ZF y  almacena en EAX el valor anterior
 * almacenado en *lock. El valor almaceando en EAX se retorna de nuevo en
 * ret.
 @return old si se pudo realizar la operacion, new si no.
*/
static __inline__ unsigned int cmpxchgl(unsigned int * lock, 
                unsigned int old,
                unsigned int new) {

    unsigned int ret;

    ret = old;

    inline_assembly("lock\n\t" \
                " cmpxchgl %2, %1" \
                : "=a" (ret), "+m" (*lock) \
                : "r" (new), "0" (old) \
                : "memory");
    return ret;
}

#endif /* ASM_H_ */
