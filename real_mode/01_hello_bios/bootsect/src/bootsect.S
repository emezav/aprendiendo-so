/**
 * @file bootsect.S
 * @ingroup bootsector_code
 * @author Erwin Meza Vega
 * @copyright MIT License
 * @brief Sector de Arranque para sistemas x86 con BIOS.
 * @description 
 * Este código ilustra el uso básico de los servicios de la BIOS. El proceso
 * general es el siguiente:

1. Escribir los caracteres 'H' 'e' 'l' 'l' 'o' en posición actual del cursor
2. Esperar por una pulsación de tecla.
3. Mover el cursor al inicio de la pantalla, es decir, a la esquina superior
   izquierda.
4. Limpiar la pantalla, escribiendo repetidamente el caracter espacio ' '. Dado
	que la pantalla en modo texto tiene 25 filas por 80 columnas, se deben
	imprimir 2000 espacios.
5. Mover de nuevo el cursor al inicio de la pantalla.
6. Escribir 'BIOS'.

  En este código se usan diferentes variaciones de la instrucción MOV, buscando
  logra una adaptación al lenguaje ensamblador y a las características del modo
  real.

*/

.intel_syntax noprefix 

.code16     

.section .text 

.global start

/**
* Símbolo que marca el inicio del código del sector de arranque 
*/
start:

	/* Escribir "Hello" en la pantalla */

	/* 'H' */
	mov ah, 0x0e /* int 0x10, ah = 0x0E Write text in teletype mode*/
	mov al, 'H' /* Caracter ASCII a imprimir */
	mov bh, 0x00 /* Primera página, modo texto */
	mov bl, 0x00 /* Ignorado, se usa en modo gráfico */
	int 0x10 /* Invocar el servicio! */

	/* 'e' */
	mov ax, 0x0e65 /* ah = 0x0E, al = 0x65 = 'e' */
	mov bx, 0x0000 /* bh = 0x00, bl = 0x00 */
	int 0x10

	/* 'l' */
	mov ax, 0x0e6c /* ah = 0x0E, al = 0x6c = 'l' */
	xor bx, bx /* Equivalente a mov bx, 0x000 */
	int 0x10

	/* 'l' */
	mov ax, 0x0e6c /* ah = 0x0E, al = 0x6c = 'l' */
	xor bx, bx /* Equivalente a mov bx, 0x000 */
	int 0x10

	/* 'o' */
	mov ax, 0x0e6f /* ah = 0x0E, al = 0x6f = 'l' */
	xor bx, bx /* Equivalente a mov bx, 0x000 */
	int 0x10

	/* Esperar una pulsación de tecla */
	xor ax, ax /* ax = 0x0000 */
	int 0x16 /* int 0x16, ah = 0x00 Wait for keystroke and read */


	/* Mover el cursor al inicio de la pantalla */
	mov ah, 0x02 
	xor bh, bh /* Página 0 */
	xor dx, dx  /* dh (fila) = 0, dl (columna) = 0 */
	int 0x10 /* int 0x10, ah = 0x02 Set cursor position */

	/* Limpiar la pantalla: Escribir 80 x 25 = 2000 espacios desde la esquina
	 * superior izquierda hasta la esquina inferior derecha */
	/* cx = 2000 */
	mov cx, 2000
cls:
	mov ax, 0x0E20 /* AH = 0x0E, AL = 0x20 = ' ' */
	xor bx, bx /* Página 0 */
	int 0x10
	loop cls /* Loop: Decrementar cx y saltar a 'cls' mientras cx > 0*/

	/* Mover de nuevo el cursor al inicio de la pantalla */
	mov ah, 0x02 
	xor bh, bh /* Página 0 */
	xor dx, dx  /* dh (fila) = 0, dl (columna) = 0 */
	int 0x10 /* int 0x10, ah = 0x02 Set cursor position */

	/* Imprimir "BIOS" en la esquina superior de la pantalla */

	/* 'B' */
	mov ax, 0x0E42
	xor bx, bx
	int 0x10

	/* 'I' */
	mov ax, 0x0E49
	xor bx, bx
	int 0x10

	/* 'O' */
	mov ax, 0x0E4F
	xor bx, bx
	int 0x10

	/* 'S' */
	mov ax, 0x0E53
	xor bx, bx
	int 0x10

	/* Fin. */ 

finished:

	hlt /* Detener el procesador hasta que ocurra una interrupción */
  
  /* Detener de nuevo el procesador. */

 jmp finished

	/**
	* El procesador nunca llegará a este punto, dado que la espera activa nunca
	* termina.
	*/
