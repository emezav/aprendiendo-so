/**
 * @file bootsect.S
 * @ingroup bootsector_code
 * @author Erwin Meza Vega
 * @copyright MIT License
 * @brief Sector de Arranque para sistemas x86 con BIOS.
 * @description 
 * Este código ilustra la configuración de los registros de segmento en
 * modo real. El proceso es el siguiente:

1. Configurar el registro de segmento CS para que contenga 0x07C0, es decir,
   para que el segmento de código inicie en la dirección lineal 0x7C00
2. Configurar los registros de segmento de datos DS, ES, FS y GS para que
   los segmentos de datos también inicien en la dirección lineal 0x7C00
3. Configurar los registros SS y ESP para que el segmento de pila tenga 
   como base 0x90000, con un tamaño inicial de 0xFC00. De esta forma, el
   tope de la pila se encontrará en la dirección lineal 0x9FC00.
*/

.intel_syntax noprefix 
.code16     

.section .text 

.global start                  
start:

  /* La ejecución comienza en este punto, con CS = 0x0000 e IP = 0x7C00 */
  /* CS = 0x7C0, IP = desplazamiento de entry_point */
  ljmp 0x7C0 : OFFSET entry_point

entry_point:
  /* La ejecución continúa en este punto, con CS = 0x07C0 e IP = 0x0005 */
  
  /* AX = CS */
  mov ax, cs
  /* DS = AX */
  mov ds, ax
  /* ES = AX */
  mov fs, ax
  /* FS = AX */
  mov fs, ax
  /* GS = AX */
  mov gs, ax

  /* Deshabilitar temporalmente las interrupciones */
  cli
  /* AX = 0x9000 */
  mov ax, 0x9000
  /* SS = AX */
  mov ss, ax
  mov sp, 0xFC00
  /* Habilitar de nuevo interrupciones */
  sti
  
  /* El tope de la pila se encuentra ahora en 0x9FC00 */

finished:
  hlt /* Detener el procesador hasta que ocurra una interrupción */
  
  /* Detener de nuevo el procesador. */
 jmp finished
