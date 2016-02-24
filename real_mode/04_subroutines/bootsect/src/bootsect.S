/**
 * @file bootsect.S
 * @ingroup bootsector_code
 * @author Erwin Meza Vega
 * @copyright MIT License
 * @brief Sector de Arranque para sistemas x86 con BIOS.
 * @description 
 * En este proyecto se muestra la definición y el uso de subrutinas.
*/

.intel_syntax noprefix 
.code16     

.section .text 

.global start
start:
  /* Configurar CS para que apunte a 0x7C00 */
  ljmp 0x7C0: offset entry_point
  
entry_point:

  /* Configurar el registro de segmento DS */
  mov ax, cs
  mov ds, ax

  /* Configurar la pila */
  cli
  mov ax, 0x9000
  mov ss, ax
  mov sp, 0xFC00
  sti
  
  /* Leer hasta 80 caracteres a la dirección 0x50: 0x0000 = 0x500 */
  /* Almacenar los párametros en la pila */
  push 80
  push 0x0000
  push 0x50
  /* getline(0x50, 0x0000, 80)*/
  call getline
  /* Quitar los parámetros de la pila */
  add sp, 6

  mov ah, 0x0E
  mov al, 0x0A
  int 0x10
  
  mov al, 0x0D
  int 0x10
  
  /* Imprimir la cadena terminada en nulo */
  /* Almacenar los párametros en la pila */
  push 0x0000
  push 0x50
  /* putline(0x50, 0x0000)*/
  call putline
  /* Quitar los parámetros de la pila */
  add sp, 6
  
finished:
  hlt /* Detener el procesador hasta que ocurra una interrupción */
  /* Detener de nuevo el procesador. */
 jmp finished

/**
* Lee un caracter de teclado usando los servicios de teclado de la BIOS
* (int 0x16).
* Entrada:
*  Ninguna
* Salida:
*  AL contiene el caracter ASCII leido
* En esta rutina no se crea un marco de pila. El servicio de la 
* BIOS puede modificar otros registros.
*/
getkey: 
  xor ah, ah   /* ah = 0, servico de video leer un caracter de teclado */
  int 0x16     /* Servicio de video */
  ret

/** 
 * Lee un máximo de caracteres por teclado o hasta leer ENTER (0x0D) a la
 * dirección lógica especificada.
 * Entrada:
 *  [bp + 4]: Selector de la dirección lógica
 *  [bp + 6]: Offset de la dirección lógica
 *  [bp + 8]: Número máximo de caracteres a leer
 * Salida:
 *  AX = Número de caracteres leidos
*/
getline:
  push bp
  mov bp, sp    /* Crear el marco de pila */

  push bx       /* Guardar temporalmente algunos registros */
  push cx
  push dx
  push di
  push es

  /* ES:DI = dirección lógica en la cual se lee la cadena */
  mov ax, [bp + 4]
  mov es, ax
  mov di, [bp + 6]

  /* CX = Máximo número de caracteres a leer */
  mov cx, [bp + 8]

  /* Ciclo de lectura */
  xor dx, dx  /* Contador DX = 0 */
1:
  cmp cx, 0 /* Terminar inmediatamente si cx es <= 0 */
  jle 2f

  call getkey /* Usar la subrutina getkey */

  /* AL = ASCII leido */
  cmp al, 0x0D   
  /* Se leyo enter? Saltar a la etiqueta anónima "3" adelante */
  je 3f

  /* En caso contrario, imprimir y almacenar el caracter en es:[di] */  
  mov ah, 0x0E /* Imprimir el caracter */
  int 0x10

  /* equivalente a: 
  mov BYTE PTR es:[di], al 
  inc di
  */
  stosb /* DI se incrementa automáticamente */
  
  inc dx /* Incrementar en 1 el contador */
  
  loop 1b   /* CX = CX - 1, ir a la etiqueta anónima "1" atrás si CX > 0 */

3:
  /* Terminar correctamente la cadena */
  mov al, 0x00
  stosb   /* Almacenar nulo (0x00) en es:di */

2:
  /* Fin de la lectura. */
  
  mov ax, dx  /* ES:DI apunta al caracter nulo de la cadena */

  pop es    /* Recuperar los valores guardados en la pila */
  pop di      
  pop dx
  pop cx
  pop bx
  
  mov sp, bp    /* Cerrar el marco de pila */
  pop bp
  ret

/** 
 * Imprime una cadena de caracteres terminada en nulo.
 * Entrada:
 *  [bp + 4]: Selector de la dirección lógica de la cadena
 *  [bp + 6]: Offset de la dirección lógica de la cadena
 * Salida:
 *  Ninguno.
*/
putline:
  push bp
  mov bp, sp    /* Crear el marco de pila */
  
  push ax
  push si
  push ds

  /* DS:SI = dirección lógica de la cadena a imprimir */
  mov ax, [bp + 4]
  mov ds, ax
  mov si, [bp + 6]

1:
  /* Equivalente a:
  mov al, BYTE PTR ds:[si]
  inc si
  */
  lodsb

  /* AL = ASCII leido */
  or al, al  
  /* El caracter es nulo? terminar */
  je 2f

  /* En caso contrario, imprimir en la pantalla */

  mov ah, 0x0E
  int 0x10

  jmp 1b

2:

  pop ds    /* Recuperar los valores guardados en la pila */
  pop si      
  pop ax 

  mov sp, bp    /* Cerrar el marco de pila */
  pop bp
  ret
