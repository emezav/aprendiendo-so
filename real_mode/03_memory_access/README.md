Descripción general del proyecto
================================
En este proyecto se ilustra el uso de los registros de segmento de datos y
algunos registros de propósito general para almacenar y leer datos en la
memoria RAM. Se lee un número arbitrario de caracteres, los cuales se
almacenan en una posición de memoria disponible por fuera del sector de
arranque. 

A medida que se leen los caracteres, se muestran por pantalla usando un
servicio de video de la BIOS. Cuando se pulsa la tecla ENTER (código 
ASCII 0x0D = Carriage Return - CR), se imprime de nuevo la cadena.

El proceso general es el siguiente:

- Configurar ES:DI para que apunten a la dirección lineal 0x500.
- Leer una cadena de caracteres usando una estructura repetitiva. Cada vez
  que se lee un carácter usando el servicio de teclado de la BIOS, 
  se almacena en la posición de memoria referenciada por ES:DI y 
  posteriormente se incrementa DI para avanzar un byte en la memoria.
- Finalmente se usa una estructura repetitiva para extraer e imprimir cada
  carácter a partir de la posición 0x500 mediante los registros DS:SI.
  
Primero se modifica el registro de segmento CS para que apunte al segmento
de memoria que inicia en 0x7C00 y se configura la pila, usando una
secuencia de instrucciones que se presentó en un ejemplo anterior.
      
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

El siguiente paso consiste en configurar los registros ES y DI para
referenciar la dirección lineal 0x500, que es la primera posición de
memoria disponible en modo real.

    
      /* Configurar ES:DI para que apunte a 0x500 */
      mov ax, 0x50
      /* ES = 0x50*/
      mov es, ax
      
      /* DI = 0x0000*/
      xor di, di 
      
      /* ES:DI = 0x500 */
      
Para leer la cadena de caracteres se usa una estructura repetitiva. Cada
vez que se lee un carácter usando el servicio de teclado de la BIOS, se 
almacena en la posición de memoria referenciada por ES:DI y posteriormente
se incrementa DI para avanzar un byte en la memoria. La estructura 
repetitiva se rompe cuando se recibe el carácter ENTER 
(0x0D en hexadecimal).

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
    

Luego de leer la cadena, se imprimen los caracteres fin de línea (LF) y
retorno de carro (CR) para avanzar una línea en la pantalla.
    
    /* Imprimir LF (0x0A) y CR (0x0D) para avanzar una línea */  
      mov ah, 0x0E  
      mov al, 0x0A
      int 0x10
      
      mov ah, 0x0E
      mov al, 0x0D
      int 0x10
      
En este punto de la ejecución, ES:DI apunta al último carácter de la cadena
leida. Para imprimir todos los caracteres de la cadena debemos usar otro
registro de segmento y otro registro de propósito general, con los cuales
apuntaremos a la dirección en la cual inicia la cadena (0x500).  En este
caso usaremos los registros DS y SI.
    
      /* Configurar DS:SI para que apunte al inicio de la cadena. */
      mov ax, es
      mov ds, ax
      xor si, si

Recordemos que en este punto de la ejecución el registro de segmento ES
contiene 0x50, es decir, apunta a un segmento de datos que inicia en 0x500.
Este valor se traslada a AX, y luego a DS.  La instrucción xor establece al
registro SI en 0x0000, por lo cual la dirección lógica DS:SI se traduce a
la dirección lineal 0x500.

El último paso consiste en usar una estructura repetitiva para extraer e
imprimir el carácter apuntado por DS:SI e incrementar SI. Cuando SI llegue
a la misma posición de DI se habrá terminado de recorrer e imprimir la
cadena, y se terminará el ciclo.
    
    /* Ciclo para imprimir. */  
    imprimir:
      /* Terminar si SI = DI, final de la cadena */
      cmp si, di
      je fin_imprimir
      
      /* al = ds:si = un caracter de la cadena */
      mov al, BYTE PTR ds:[si]
      
      /* Imprimir el caracter por pantalla */
      mov ah, 0x0E
      int 0x10
      
      /* Avanzar en la posición de la cadena */
      inc si
      
      /* Repetir */
      jmp imprimir  
      
    fin_imprimir:

Compilación y ejecución del proyecto
==================================

La compilación del código y la ejecución del emulador se realiza mediante la
utilidad *make*. Para ejecutar este proyecto, se debe abrir un *shell* y
ubicarse en la carpeta del proyecto. Luego se deberá ingresar uno de los
siguientes comandos, de acuerdo con el emulador que se desee usar para ejecutar
el código:
- __make qemu__: Compila el código, crea la imagen de disco floppy e Inicia el
  emulador *qemu* con la imagen de disco creada.
- __make bochs__: Similar al comando anterior, pero a cambio inicia el emulador
   *bochs*.
- __make bochsdbg__: En este caso se compila el código, se crea la imagen de
	disco floppy y se inicia el emulador *bochs* desde esta imagen de disco, con
	la interfaz gráfica del depurador activada.

También se cuenta con otros *targets* que permiten realizar otras tareas:
- __make clean__: Elimina los archivos resultado de la compilación del código y
	la imagen de disco obtenida.
- __make docs__: Genera automáticamente la documentación del proyecto en formato
	html y latex, dentro del directorio *docs*.

Puede revisar el archivo __Makefile__ que se encuentra en el directorio del
proyecto para estudiar el proceso que realiza la utilidad *make*.

Depuración paso a paso
----------------------
En el archivo bochsrc.txt se ha activado el *Magic break*, por lo cual si se
incluye la instrucción *xchg bx, bx* en cualquier parte del código, se pausará
la ejecución cuando se usa el emulador bochs con el depurador gráfico activado
(comando *make bochsdbg*).


Vea también
===========
- David Jurgens, Help-PC Reference Library http://stanislavs.org/helppc/idx_interrupt.html
