/**
 * @file bootsect.S
 * @ingroup bootsector_code
 * @author Erwin Meza Vega
 * @copyright MIT License
 * @brief Sector de Arranque para sistemas x86 con BIOS.
 * @description 
 * El siguiente ejemplo muestra cómo leer un número arbitrario de
 * caracteres por teclado, almacenando los caracteres leídos en una
 * posición de memoria disponible. 
 * A medida que se leen los caracteres, se muestran por pantalla usando el
 * servicio de video de la BIOS. Cuando se pulsa la tecla ENTER 
 * (código ASCII 0x0D = Carriage Return - CR), se imprime de nuevo la
 * cadena. 
*/

.intel_syntax noprefix 
.code16     

.section .text 

.global start
start:
  /* Configurar CS para que apunte a 0x7C00 */
  ljmp 0x7C0: offset entry_point
  
entry_point:

  /* Configurar la pila */
  cli
  mov ax, 0x900
  mov ss, ax
  mov sp, 0xFC00
  sti
  
  /* Configurar ES:DI para que apunte a 0x500 */
  mov ax, 0x50
  /* ES = 0x50*/
  mov es, ax
  
  /* DI = 0x0000*/
  xor di, di 
  
  /* ES:DI = 0x500 */
  
  /* Máximo de caracteres */
  mov cx, 80

/* Ciclo de lectura. */
leer:  

  /* Leer un caracter usando el servicio de la BIOS */
  xor ax, ax  
  int 0x16
  
  /* Comparar con ENTER = 0x0D = CR */
  cmp al, 0x0D

  /* Si es igual, terminar el ciclo */
  je fin_leer
  
  /* Almacenar el caracter leido a ES:DI */
  mov BYTE PTR es:[di], al
  
  /* Incrementar el apuntador a la memoria */
  inc di
  
  /* Imprimir el caracter que se encuentra en AL*/
  mov ah, 0x0E
  int 0x10
  
  /* Decrementar CX y saltar a lectura, mientras CX > 0. */
  loop leer
  
fin_leer:

/* Imprimir LF (0x0A) y CR (0x0D) para avanzar una línea */  
  mov ah, 0x0E  
  mov al, 0x0A
  int 0x10
  
  mov ah, 0x0E
  mov al, 0x0D
  int 0x10
  
  /* Configurar  DS:SI para que apunte a 0x500 */
  mov ax, es
  mov ds, ax
  xor si, si
 
/* Ciclo para imprimir. */  
imprimir:
  /* Terminar si SI = DI, final de la cadena */
  cmp si, di
  je fin_imprimir
  
  /* al = ds:si = caracter de la cadena */
  mov al, BYTE PTR ds:[si]
  
  /* Imprimir el caracter por pantalla */
  mov ah, 0x0E
  int 0x10
  
  /* Avanzar en la posición de la cadena */
  inc si
  
  /* Repetir */
  jmp imprimir  
  
fin_imprimir:
  

finished:
  hlt /* Detener el procesador hasta que ocurra una interrupción. */
  /* Detener de nuevo el procesador. */
 jmp finished
