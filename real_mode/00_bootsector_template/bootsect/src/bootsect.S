/**
 * @file bootsect.S
 * @ingroup bootsector_code
 * @author Erwin Meza Vega
 * @copyright MIT License
 * @brief Sector de Arranque para sistemas x86 con BIOS.
 * @description 
 * Este es un sector de arranque válido.
*/

/** 
 * Directiva para el compilador. Sintaxis Intel, sin prefijo para el nombre de
 * los registros 
 */
.intel_syntax noprefix 

/** 
 * Directiva para el compilador. Código de 16 bits para ejecutarse en modo real
 */
.code16     

/** 
* Directiva para el compilador. Sección de código que contendrá las
* instrucciones a ejecutar.
*/
.section .text 

/** 
 * Directiva para el compilador/ linker. El símbolo start es global, necesario
 * para el linker (Ver link_bootsector.ld)
 */
.global start                  

/**
* Símbolo que marca el inicio del código del sector de arranque 
*/
start:

	/* Esta instrucción es un "magic breakpoint" para el emulador bochs cuando
	 * se ejecuta en modo de depuración. El depurador se detendrá cada
	 * vez que la encuentre, lo cual nos dará la oportunidad de revisar el
	 * estado de la CPU. 
	 */
	xchg bx, bx

	/* La instrucción anterior intercambia el valor almacenado en bx consigo
	 * mismo, por lo cual no causa ningún problema en otro emulador o en
	 * hardware real. 
	 */

finished:

	/* Mantener el procesador en un ciclo*/

	/* Al llegar a este punto, se entra en una "espera activa", es decir,
	* mantenemos al procesador ejecutando la misma instrucción hasta que la
	* máquina sea apagada o reiniciada. */

	/* Aún si ocurre una interrupción, la BIOS la atenderá y luego regresará a
	 * este mismo punto. */

 jmp finished

	/**
	* El procesador nunca llegará a este punto, dado que la espera activa nunca
	* termina.
	*/
