/**
 * @file bootsect.S
 * @ingroup bootsector_code
 * @author Erwin Meza Vega
 * @copyright MIT License
 * @brief Sector de Arranque para sistemas x86 con BIOS.
 * @description
 * Este código se compila en un sector de arranque que es cargado
 * automáticamente por la BIOS en la posición de memoria 0x7C00. La BIOS
 * transfiere el control de la ejecución a este código, que simplemente entra en
 * una espera activa para mantener ocupado el procesador.
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

	wait_kbd:
    in al, 0x64       /* Leer registro de estado del 8042 */
    test al, 0x01     /* Verificar bit 0 (Output Buffer Full) */
    jz wait_kbd       /* Si bit 0 = 0, seguir esperando */

	movb ah, 0x0E   /* Función teletype de escritura de caracteres */
	movb al, 'X'    /* El carácter a imprimir */
	movb bh, 0      /* Página de display */
	int 0x10

	/* La instrucción anterior intercambia el valor almacenado en bx consigo
	 * mismo, por lo cual no causa ningún problema en otro emulador o en
	 * hardware real.
	 */

finished:

   /* Detener el procesador con la instrucción HLT. Si las interrupciones
   se encuentran habilitadas, la BIOS toma el control, atiende la
   interrupción y continúa justo después de la instrucción HLT. */

   hlt

	/* LA BIOS regresará a este punto. Detener de nuevo el procesador! */

 jmp finished

	/**
	* El procesador nunca llegará a este punto, dado que la espera activa nunca
	* termina.
	*/
