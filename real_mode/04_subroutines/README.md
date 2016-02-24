Descripción general del proyecto
================================
En modo real se puede definir y usar subrutinas. Vamos a definir algunas
que luego las usaremos en el código principal del sector de arranque:

1. Leer un caracter por teclado: Esta subrutina no recibe ningún parámetro
   de entrada. Mediante los servicios de teclado de la BIOS leerá un
   caracter, y éste será retornado en el registro AL.
2. Leer una cadena de caracteres por teclado: Esta subrutina recibirá como
   parámetro la dirección lógica (selector:offset) de la ubicación de
   memoria en la cual se desea leer la cadena, y un número entero que
   representa el número máximo de caracteres a leer. Esta subrutina deberá
   adicionar el caracter nulo (0x00) después del último caracter leido.
3. Imprimir una cadena de caracteres: Recibe como parámetro la dirección
   lógica de la cadena a imprimir. El fin de la cadena deberá estar marcado
   con el caracter nulo.

Leer un caracter por teclado
----------------------------

Para leer un caracter por teclado solo se necesita almacenar 0x00 en el
registro AL e invocar el servicio de la BIOS proprcionado por la
interrupción 0x16 (servicios de teclado). Este servicio almacena el código
ASCII en el registro AL, y el código de escaneo en el registro AH. 

    /**
    * Lee un caracter de teclado usando los servicios de teclado de la BIOS
    * (int 0x16).
    * Entrada:
    *  Ninguna
    * Salida:
    *  ah contiene el Codigo de Escaneo leido
    *  al contiene el caracter ASCII leido
    * En esta rutina no se crea un marco de pila. El servicio de la 
    * BIOS puede modificar otros registros.
    */
    getkey: 
      xor ah, ah   /* ah = 0, servico de video leer un caracter de teclado */
      int 0x16       /* Servicio de video */
      ret

En este caso no se creará el marco de pila dentro de la rutina, pero se
debe verificar que al ejecutarse no modifique los valores de otros
registros generales o de segmento.  Una implementación más “segura”, en la
cual se crea un marco de pila y se almacenan temporalmente los registros de
propósito general se muestra a continuación.

    /**
    * Lee un caracter de teclado usando los servicios de teclado de la BIOS
    * (int 0x16).
    * Entrada:
    *  Ninguna
    * Salida:
    *  ah contiene el Codigo de Escaneo leido
    *  al contiene el caracter ASCII leido
    * En esta rutina crea un marco de pila y almacena temporalmente en la
    * pila los valores de los registros antes de invocar el servicio de la
    * BIOS, y luego los recupera de la pila.
    */
    getkey:
      push bp
      mov bp, sp
      
      push bx
      push cx
      push dx
      push si
      push di
      
      xor ah, ah   /* ah = 0, servico de video leer un caracter de teclado */
      int 0x16       /* Servicio de video */
      
      pop di
      pop si
      pop dx
      pop cx
      pop bx
      
      mov sp, bp
      pop bp
        
      ret

En este caso no se puede usar la instrucción pusha para almacenar los
valores de todos los registros de propósito general antes de invocar el
servicio y popa para recuperarlos,  debido a que se perdería el valor de AX
en el cual se ha leído el caracter.

Es necesario depurar este código, para verificar si el servicio modifica
otros registros. En caso que no se modifique ninguno, la primera solución
presentada es suficiente.

Leer una cadena de caracteres por teclado
-----------------------------------------

Esta subrutina recibe tres parámetros por la pila: la dirección lógica
(selector y offset) a la cual se leerá la cadena, y el número máximo de
caracteres a leer. De acuerdo con lo presentado en el capítulo 2, la
ubicación de estos parámetros en la pila luego de crear el marco
correspondiente será:

- [bp + 4]: Selector de la dirección lógica.
- [bp + 6]: Offset de la dirección lógica.
- [bp + 8]: Número máximo de caracteres a leer.

Para tratar que la subrutina tenga el menor impacto posible, se almacenarán
temporalmente en la pila los registros usados y luego se recuperarán antes
de retornar.

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
    
      push cx       /* Guardar temporalmente algunos registros */
      push dx
      push di
      push es

Luego se extraen los parámetros de la pila. La dirección lógica se
alamcenará en ES:DI, y el número máximo de caracteres a leer en CX.

    /* ES:DI = dirección lógica en la cual se lee la cadena */
    mov ax, [bp + 4]
    mov es, ax
    mov di, [bp + 6]
  
    /* CX = Máximo número de caracteres a leer */
    mov cx, [bp + 8]

El registro CX permite controlar la estructura repetitiva que realiza la
lectura. En cada iteración se invoca la subrutina getkey presentada
anteriormente

    /* Ciclo de lectura */
    1:
    /* Iteración para leer un caracter */
    call getkey /* Usar la subrutina getkey */
  
    /* El caracter leido se encuentra en AL, almacenar en ES:DI y avanzar DI */
  
    /* equivalente a: 
    mov es:[di], al 
    inc di
    */
    stosb /* DI se incrementa automáticamente */
  
    loop 1b   /* CX = CX - 1, ir a la etiqueta anónima "1" atrás si CX > 0 */

Este ciclo básico permitirá como máximo el número de caracteres
especificado en CX, sin embargo, nos falta tener en cuenta las siguientes
consideraciones:
- Se debe imprimir cada caracter a medida que se realiza la lectura.
- La lectura también se puede terminar cuando el usuario ha presionado
  ENTER (0x0D). Este carácter no se debe imprimir.
- Se debe contar el número de caracteres leido. En este caso se usará el
  registro DX. Este valor se trasladará al registro AX antes de retornar de
  la subrutina.
- Al terminar la lectura se debe terminar correctamente la cadena, es
  decir, adicionar el caracter nulo (0x00) al final de la misma.

El código modificado es el siguiente:

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

Al terminar la lectura se debe recuperar de la pila los valores de los
registros que se han modificado, y almacenar en el registro AX el valor del
contador del número de caracteres leidos.

    mov ax, dx  /* ES:DI apunta al caracter nulo de la cadena */
  
    pop es    /* Recuperar los valores guardados en la pila */
    pop di      
    pop dx
    pop cx
    
    mov sp, bp    /* Cerrar el marco de pila */
    pop bp
    ret

Imprimir una cadena de caracteres terminada en nulo
---------------------------------------------------

Esta subrutina recibe como parámetro la dirección lógica en la cual inicia
la cadena a imprimir. Mediante una estructura repetitiva, se imprime uno
por uno los caracteres de la cadena, hasta llegar al caracter nulo.

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

Al finalizar la estructura repetitiva, se recuperan los valores de los
registros de propósito general y se cierra el marco de pila.

    pop ds    /* Recuperar los valores guardados en la pila */
    pop si      
    pop ax 
  
    mov sp, bp    /* Cerrar el marco de pila */
    pop bp
    ret

Uso de las subrutinas 
---------------------

El código principal del sector de arranque debe primero configurar los
registros de segmento CS y DS para apuntar a 0x7C00, así como configurar la
pila.

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

Posteriormente se  invoca la rutina  para leer una cadena de caracteres a
la dirección 0x500.

    /* Leer hasta 80 caracteres a la dirección 0x50: 0x0000 = 0x500 */
    /* Almacenar los parametros en la pila */
    push 80
    push 0x0000
    push 0x50
    /* getline(0x50, 0x0000, 80)*/
    call getline
    /* Quitar los parámetros de la pila */
    add sp, 6
 
Se debe recordar que los parámetros se insertan en la pila en el orden
inverso, para que el último parámetro insertado (en este caso el selector)
se encuentre en la posición [bp + 4] en la subrutina, luego de configurar
el marco de pila.

También se debe tener en cuenta que después de ejecutar la subrutina se
deben extraer los parámetros de la pila, adicionando tantos bytes como se
insertaron con los parámetros.

Se puede probar la subrutina de imprimir una cadena pasando como parámetro
la misma dirección lógica en la cual se realizó la lectura, en este caso
0x500. Antes se debe imprimir los caracteres de fin de línea y retorno de
carro para avanzar una línea en la pantalla.

    mov ah, 0x0E
    mov al, 0x0A
    int 0x10
    
    mov al, 0x0D
    int 0x10
    
    /* Imprimir la cadena terminada en nulo */
    /* Almacenar los parametros en la pila */
    push 0x0000
    push 0x50
    /* putline(0x50, 0x0000)*/
    call putline
    /* Quitar los parámetros de la pila */
    add sp, 6


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
