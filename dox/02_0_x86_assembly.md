
Fundamentos de lenguaje ensamblador
====================================

Sintaxis AT&T e Intel en Lenguaje Ensamblador
---------------------------------------------


Si bien la sintaxis Intel es la más utilizada en la documentación y los
ejemplos, es conveniente conocer también los aspectos básicos de la sintaxis
AT&T, ya que es la sintaxis por defecto del ensamblador de GNU. Las sintaxis
AT&T e Intel difieren en varios aspectos, entre ellos:

- Nombre de los registros. En sintaxis AT&T los registros (de 16 bits) se
	denominan: %ax, %bx, %cx, %dx, %si, %di, además de los registros %sp, %bp, y
	los registros de segmento %cs, %ds, %es, %fs y %gs. Por su parte, en la
	sintaxis Intel (con la directiva '.intel_syntax noprefix') no es necesario
	anteponer '' al nombre de los registros.
- Orden de los operandos: En la sintaxis AT&T, el orden de los operandos en las
	instrucciones es el siguiente:  instrucción fuente, destino Mientras que en la
	sintaxis Intel, el orden de los operandos es:  instrucción  destino, fuente
- Operandos inmediatos. Los operandos inmediatos son aquellos valores que se
	especifican de forma explícita dentro de la instrucción. En la sintaxis AT&T,
	los operandos inmediatos se preceden con el signo '$'. En sintaxis Intel, no
	es necesario.
- Longitud de operaciones de memoria: En la sintaxis AT&T, cada instrucción debe
	llevar un sufijo que indica la longitud de la operación de memoria (movb
	para byte, movw para word, movl para long). En sintaxis Intel sólo se debe
	especificar el sufijo en aquellas instrucciones cuyos operandos son
	direcciones de memoria o cuando existe ambiguedad en el tamaño de los
	operandos.  


Referencia implícita a los registros de segmento
------------------------------------------------

Recuerde que sin importar el modo de operación del procesador, siempre se usa
segmentación. Así que las direcciones de memoria especificadas en una
instrucción de ensamblador siempre serán relativas al inicio de un segmento. Las
instrucciones de movimiento de datos (mov, lods, stos, etc) siempre harán
referencia implícita al segmento de datos apuntado por el registro de segmento
DS. Las instrucciones de salto siempre harán uso implícito del registro de
segmento CS, y las operaciones sobre la pila (push, pop) o sobre los registros
ESP y EBP siempre harán referencia al segmento apuntado por el registro SS.

En algunas instrucciones es posible cambiar el registro de segmento al cual se
hace referencia de forma implícita.

Por ejemplo, la instrucción

    mov al, [addr]


Es interpretada por el procesador como 

    mov al, ds:[addr]

Y la instrucción 



Es intepretada por el procesador como: 

    mov word ptr es:[di], word ptr ds:[si]
    inc si
    inc di

Así es necesario revisar la documentación de las instrucciones de ensamblador
para determinar si es necesario especificar de forma explícita el registro de
segmento.

Tamaño de los operandos
-----------------------

El uso de las instrucciones de ensamblador depende en gran medida del tipo de
programa que se desea desarrollar. Si se planea desarrollar un programa para ser
ejecutado en un entorno de 16 bits (Modo Real), solo se puede tener acceso a los
16 bits menos significativos de los registros, y a un conjunto limitado de
instrucciones.

Por el contrario, si se planea desarrollar un programa para ser ejecutado en un
entorno de modo protegido de 32 (o 64) bits, se puede tener acceso a la
totalidad de bits de los registros y a un conjunto mayor de instrucciones.

La mayoría de instrucciones presentadas a continuación realizan operaciones a
nivel de bytes (por ello tienen el sufijo ‘b’), y para el acceso a memoria se
utiliza BYTE PTR para indicar apuntadores a bytes. También es posible operar a
nivel de words (2 bytes, 16 bits), usando el sufijo ‘w’ y el modificador WORD
PTR en vez de BYTE PTR para apuntadores de tipo word en memoria en sintaxis
Intel.

En modo de 32 bits, es posible operar sobre doublewords (4 bytes, 32 bits)
usando el sufijo ‘w’, y para instrucciones de acceso a memoria se utiliza en
sintaxis Intel se debe especificar DWORD PTR en vez de BYTE PTR, o usar el
sufijo ‘l’ en la instrucción mov. En operaciones de 64 bits se debe usar el
sufijo ‘q’ (quadword, 8 bytes, 64 bits) para la mayoría de instrucciones y QWORD
PTR para el acceso a memoria en sintaxis Intel.

Se debe recordar que el uso de los registros depende del modo de operación del
procesador. Así, en modo real se puede tener acceso a los primeros de 8 bits y
16 bits de los registros de propósito general, en modo protegido se puede
acceder a los 32 bits de los registros y en modo de 64 bits se puede tener
acceso a los 64 bits de los registros.

Instrucciones para el movimiento de datos
=========================================

INSTRUCCIÓN mov (move)
----------------------

Permite mover (copiar) datos entre dos registros, de un valor inmediato a un
registro o de un valor inmediato a una posición de memoria.

En sintaxis AT&T:
                 
De inmediato a registros

    movb $0x10, %ah /* Mover el byte (valor inmediato) 0x10 a %ah */
    movb $0x20, %al /* Mover el byte 0x20 a %al */
    movw $0x1020, %ax /* Mueve un word (2 bytes) a %ax */
    movl $0x00001020, %eax /* Mueve un doubleword (4 bytes) a %eax */
    movq $0x000000000001020, %rax /* Mueve un quadword (8 bytes) a %rax */


De registros a registros

    movb %al, %bl /* Mover un byte de %al a %bl */
    movb %ah, %bh  /* Mover un byte de %ah a %bh */
    movw %ax, %bx /* Equivalente a las dos instrucciones anteriores */
    movl %eax, %ebx /* 32 bits */
    movw %rax, %rbx /* 64 bits */


De registros a memoria
    
    movb %al, (%si) /* Mover el byte almacenado en %al a la posición 
       de memoria  apuntada por %si */
    movb %al, 4(%si) /* Mover el byte almacenado en %al a la posición 
    de memoria apuntada por (%si + 4)*/
    movb %al, -2(%si) /* Mover el byte almacenado en %al a la posición 
       de memoria apuntada por (%si - 2) */
    movw %ax, (%si) /* Mover el word almacenado en %ax a la posición
     de memoria  apuntada por %si */
    movl %eax, (%esi) /* 32 bits */ 
    movq %rax, (%rsi) /* 64 bits */


De memoria a registros

    movb (%si), %al  /* Mover el byte de la posición de memoria
     apuntada por (%si) a %al */
    movb 4($si), %al /* Mover el byte de la posición de memoria 
       apuntada por (%si + 4) a %al */ 
    movb -2($si), %al /* Mover el byte de la posición de memoria
     apuntada por (%si - 2) a %al */

En sintaxis Intel:

El orden de los operandos es instrucción destino, fuente

De inmediato a registros

    mov ah, 0x10 /* Mover el byte (valor inmediato) 0x10 a ah */
    mov al, 0x20 /* Mover el byte 0x20 a al */
    mov ax, 0x1020 /* Equivalente a las dos operaciones anteriores */
    mov eax, 0x00001020 /* 32 bits */
    mov rax, 0x0000000000001020 /* 64 bits */


De registros a registros

    mov bl, al /* Mover un byte de al a bl */
    mov bh, ah  /* Mover un byte de ah a bh */
    mov bx, ax /* Equivalente a las dos instrucciones anteriores */
    mov ebx, eax /* 32 bits */
    mov rbx, rax /* 64 bits */


De registros a memoria

    mov BYTE PTR [ si ], al /* Mover el byte almacenado en al a la 
        posicion de memoria apuntada por si */
    movb [ si ], al      /* Equivalente a la instrucción anterior. 
          Observe el sufijo ‘w’ en la instrucción. */
    
    mov BYTE PTR [ si + 4 ], al /* Mover el byte almacenado en %al a 
       la posición de memoria apuntada por (si + 4)*/
           
    mov BYTE PTR [ si - 2 ], al /* Mover el byte almacenado en %al a 
    la posición de memoria apuntada por (si - 2) */
    
    mov WORD PTR [ si ], ax /* Mover el word almacenado en ax a la
    posición de memoria apuntada por (si) */  
    
    mov DWORD PTR [ esi ], eax /* 32 bits */ 
    mov QWORD PTR [ rsi ], rax /* 64 bits */ 


De memoria a registros

    mov al, BYTE PTR [ si ]   /* Mover el byte de la posición de 
    memoria apuntada por (si) a %al */
    movw al, [ si ]   /* Equivalente a la instrucción anterior */
    
    mov al, BYTE PTR [ si + 4] /* Mover el byte de la posición de 
    memoria apuntada por (si + 4) a al */ 
    
    mov al, BYTE PTR [ si - 2 ] /* Mover el byte de la posición de 
    memoria apuntada por (si - 2) a al */
    
    mov ax, WORD PTR [ si ]   /* Mover un word */
    mov eax, DWORD PTR [ esi ]   /* 32 bits (doubleword)*/
    mov rax, QWORD PTR [ rsi ]   /* 64 bits (quadword) */


INSTRUCCIÓN movs (move string)
------------------------------


Permite copiar datos entre dos posiciones de memoria. Automáticamente incrementa
los dos apuntadores, luego de la copia (Ver Repeticiones).

En sintaxis AT&T

    movsb (%si), (%di) /* Copia un byte de la posición de memoria apuntada  
                           por el registro %si a la posición de memoria 
                           apuntada por el registro %di */
    movsw (%si), (%di) /* Copia un word de (%si) a (%di) */
    movsl (%esi), (%edi) /* Copia un dobuleword de (%esi) a (%edi) */
    movsq (%rsi), (%rdi) /* Copia un quadword de (%esi) a (%edi) */


En sintaxis Intel

    mov BYTE PTR  [ si ], BYTE PTR [ di ] /* Mueve un byte de (si) a (di)*/
    movb [ si ], [ di ] /* Equivalente a la instrucción anterior */
    mov WORD PTR [ si ], WORD PTR [ di ] /* Mueve un word de (si) a (di) */
    mov DWORD PTR [ esi ], DWORD PTR [ esi ] /* 32 bits */
    mov QWORD PTR [ rsi ], QWORD PTR [ rdi ] /* 64 bits */


INSTRUCCIÓN lods (load string)
------------------------------


Permite copiar datos entre una posición de memoria y un registro. Automáticamente incrementa el apuntador a la memoria en el número de bytes de acuerdo con la longitud de operación.

En sintaxis AT&T

    lodsb /* También es valido lodsb %al, (%si) */
    /* Almacena un byte de la posición de memoria apuntada por (%si)en en el registro %al */
      
    lodsw /* Almacena un word de la posición de memoria apuntada por 
     (%si) en el registro %ax */
    
    lodsl /* 32 bits */
    
    lodsq /* 64 bits */


En sintaxis Intel

    lods al, BYTE PTR [ si ]    /* Almacena un byte de la posición de memoria 
                               apuntada por (si) en  el registro al */
    lodsb    /* Equivalente a la instrucción anterior. La sintaxis abreviada también es válida */
      
    lods ax, WORD PTR [ si ] /* Almacena un word de la posición de
                                memoria apuntada por (si) en el registro ax */
                                
    lodsw   /* Equivalente a la instrucción anterior. */
    
    lodsl /* 32 bits */
    
    lodsq /* 64 bits */
    
    INSTRUCCIÓN stos (store string)


Permite copiar datos entre un registro y una posición de memoria. Incrementa automáticamente el apuntador a la memoria.

En sintaxis AT&T

    stosb /* También es valido stosb %al, (%di) */
    /* Almacena el valor de %al a la posición de memoria apuntada por (%di)  */
    stosw /* Almacena el valor de %ax a la posición de memoria
     apuntada por (%di) */
    
    stosl /* 32 bits */
    
    stosq /* 64 bits */


En sintaxis Intel

    stos BYTE PTR [ di ], al
       /* Almacena el valor de al a la posición de memoria 
      apuntada por   (di) */
    
    stosb   /* También es válida la instrucción abreviada */
      
    stos WORD PTR [ di ], ax  /* Almacena el valor de ax a la posición 
    de memoria apuntada por  (di) */
    stosw  /* Equivalente a la instrucción anterior. */
    
    stosl /* 32 bits */
    
    stosq /* 64 bits */


Repeticiones
------------

Las instrucciones movs, lods y stos pueden hacer uso del prefijo 'rep' (repeat), para repetir la operación incrementando los registros ESI o EDI sea el caso y la longitud de la operación, mientras el valor del registro ECX sea mayor que cero. El valor de ECX debe ser establecido antes de invocar la instrucción.

Por ejemplo, las siguiente secuencia de instrucciones en sintaxis AT&T  copia el valor del registro AL (un byte) o AX (dos bytes) a la posición de memoria apuntada por ES:(DI), y automáticamente incrementa el apuntador DI. Cada vez que se ejecuta la instrucción, el registro CX se decrementa y se compara con cero. Los cuatro ejemplos realizan la misma acción en modo real: permiten copiar 256 bytes de un registro a la memoria.

En la primera forma se realizan 256 iteraciones (0x100) para copiar byte a byte, y en la segunda solo se requieren 128 iteraciones (0x80), ya que cada vez se copia un word (dos bytes) cada vez.

    movw $0x100, %cx 
    rep stosb /* 16 bits, copiar byte a byte, incrementar %di en 1*/
    
    movw $0x80, %cx 
    rep stosw /* 16 bits, copiar word a word, incrementar %di en 2 */


en sintaxis Intel:

    mov cx, 0x100
    rep stosb BYTE PTR [ di ], al /* 16 bits, copiar byte a byte */
    
    mov cx, 0x80
    rep stosw WORD PTR [ di ], ax /* 16 bits, copiar word a word */



Dirección de incremento
----------------------


Se debe tener en cuenta que las instrucciones lods, stos y movs automáticamente incrementan los registros ESI o EDI según sea el caso y la longitud de la operación (byte, word, doubleword o quadword).

Esto se puede garantizar invocando la instrucción cld (clear direction flag) con la cual se realiza el incremento automáticamente.

Por el contrario, la instrucción std (set direction flag) causa que las instrucciones decrementen automáticamente los registros ESI o EDI según sea el caso.

 
Saltos
=======

Para evitar la linealidad de la ejecución, el procesador dispone de instrucciones que permiten 'saltar' (cambiar el valor de EIP), para poder continuar la ejecución del programa en otro sitio diferente dentro del código. A continuación se describe de forma resumida las instrucciones más usadas para realizar saltos dentro del código.

Instrucción jmp (jump)
-----------------------


Salto incondicional. Permite continuar la ejecución de forma incondicional en otra posición de memoria, designada generalmente por una etiqueta en el código fuente ensamblador.

Ejemplo (En sintaxis AT&T e Intel ):

    label1:
     ...
    (instrucciones)
    ...
    jmp label2
    (xxxinstruccionesxxx)
    ...
    label2:
    ...
    (instrucciones)
    ...


En este ejemplo, las instrucciones desde jmp label2 hasta la etiqueta label2 no son ejecutadas. Esto es especialmente útil si se requiere ‘saltar’ una región del código en ensamblador.

En los programas jmp generalmente se salta dentro del mismo segmento. No obstante, si programa lo requiere, es posible saltar a otros segmentos.

Saltos entre segmentos


El formato de la instrucción JMP para salto entre segmentos es la siguiente:

En sintaxis AT&T:

    ljmp seg, offset

En sintaxis Intel:
 
    ljmp seg:offset


En estos formatos de jmp, CS adquiere el valor especificado en 'seg' y EIP adquiere el valor especificado en 'offset'.

Un ejemplo clásico consiste en el salto inicial que debe realizar un sector de arranque, para garantizar que el registro de segmento CS contenga el valor de 0x7C0. Si bien la BIOS siempre carga al sector de arranque en la Dirección lineal 0x7C00, es necesario establecer CS explícitamente en 0x7C0 para que apunte al inicio del segmento de código del sector de arranque cargado en memoria.

Esto se logra mediante la siguiente estructura de código:

    start:
       ljmp 0x7C0 : OFFSET entry_point
    
    
    entry_point: /* La ejecución continúa en este punto, pero CS = 0x7C0. */
       //demás instrucciones
       
       
       //..
       
       jmp finished
       
    finished: /* Ciclo infinito dentro del código del sector de arranque */
       jmp finished 

Instrucciones de salto condicional
------------------------------------- 


Instrucciones jz, jnz, je, jne, jle, jge, jc. Estas instrucciones generalmente vienen precedidas por instrucciones que realizan manipulación de registros o posiciones de memoria, y que alteran el contenido del registro EFLAGS (alguno o varios de sus bits).

La sintaxis de todas estas instrucciones es la misma: 

    jz label
    jnz label
    je label
    jne label
    jge label


Con estas instrucciones se realiza un salto a la etiqueta __label__ en el código ensamblador, dependiendo si se cumple o no la condición de acuerdo con la instrucción. Si la condición no se cumple, el procesador continúa ejecutando la siguiente instrucción que se encuentre después de la instrucción de salto condicional.

Las instrucciones y condiciones más importantes son:
- jz / je label (jump if zero / jump if equal): Saltar a la etiqueta label si el bit ZF del registro EFLAGS se encuentra en 1, o si en una comparación inmediatamente anterior los operandos a comparar son iguales.
- jnz / jne label (jump if not zero / jump if not equal): Contrario a jz. Saltar a la etiqueta label si el bit ZF del registro EFLAGS se encuentra en 0, o si en una comparación inmediatamente anterior los operandos a comparar no son iguales.
- jc label: Saltar a la etiqueta label si el bit CF del registro FLAGS se encuentra en 1. Este bit se activa luego de que se cumplen determinadas condiciones al ejecutar otras instrucciones, tales como sumas con números enteros. Igualmente algunos servicios de DOS o de la BIOS establecen el bit CF del registro FLAGS en determinadas circunstancias.
- jnc label: Contrario a jc. Saltar a la etiqueta si el bit CF del registro EFLAGS se encuentra en 0.

Saltos condicionales y comparaciones
-------------------------------------

Uno de los usos más comunes de las instrucciones de salto condicional es comparar si un operando es mayor o menor que otro para ejecutar código diferente de acuerdo con el resultado. Para ello, las instrucciones de salto condicional van precedidas de instrucciones de comparación:
 
    cmp fuente, destino  /*(AT&T)*/


ó

    cmp destino, fuente  /* (Intel) */


Otras instrucciones de salto condicional que también se pueden utilizar para números con signo son:
- jg label: jump if greater: Saltar a la etiqueta label si la comparación con signo determinó que el operando de destino es mayor que el operando de fuente
- jl label: jump if less: Saltar a la etiqueta label si la comparación encontró que el operando de destino es menor que el operando de fuente
- jge label: Jump if greater of equal: Saltar a la etiqueta label si el operando de destino es mayor o igual que el operando de fuente
- jle: Jump if less or equal : Saltar a la etiqueta label si el operando de destino es menor o igual que el operando fuente.

EJEMPLOS
----------


En el siguiente ejemplo se almacena el valor inmediato 100 en el registro AL y
luego se realiza una serie de comparaciones con otros valores inmediatos y
valores almacenados en otros registros.

En sintaxis AT&T: 

      movb $100, %al /* A los valores inmediatos en decimal  no se les antepone '0x'
            como a los hexa*/
      cmpb $50, %al /* Esta instrucción compara el valor de %al  con 50 */
      jg es_mayor
      /* Otras instrucciones, que se ejecutan si el valor de %al no es mayor que 50
       (en este caso no se ejecutan, %al = 100 > 50 */
      jmp continuar
     /* Este salto es necesario, ya que de otro modo el procesador 
      ejecutará las instrucciones anteriores y las siguientes 
      también, lo cual es un error de lógica*/
      es_mayor:
       /*  La ejecución continua aquí si el valor de %al (100) es mayor  que 50*/
      continuar:
         /*  Fin de la comparación. El código de este punto hacia 
      Abajo se ejecutará para cualquier valor de %al */


En sintaxis Intel: 

       mov al, 100  /* A los valores inmediatos en decimal  no se les antepone '0x'
                     como a los hexa*/
      cmpb al, 50 /* Esta instrucción compara el valor de %al  con  50*/
      jg es_mayor
      /* Otras instrucciones, que se ejecutan si el valor de %al no es mayor que 50 
       (en este caso no se ejecutan, %al = 100 > 50 */
      jmp continuar
     /* Este salto es necesario, ya que de otro modo el procesador 
      ejecutará las instrucciones anteriores y las siguientes 
      también, lo cual es un error de lógica*/
      es_mayor:
     /*  La ejecución continua aquí si el valor de %al (100) es mayor  que 50*/
      continuar:
      /*  Fin de la comparación. El código de este punto hacia 
      Abajo se ejecutará para cualquier valor de %al */

Ciclos
======

Los ciclos son un componente fundamental de cualquier programa, ya que permiten repetir una serie de instrucciones un número determinado de veces. Existen varias formas de implementar los ciclos. Las dos formas más acostumbradas son:
1.Combinar un registro que sirve de variable, una comparación de este registro y una instrucción de salto condicional para terminar el ciclo
2.Usar la instrucción loop y el registro CX. Este registro no se puede modificar dentro del cuerpo del ciclo.

A continuación se ilustran los dos casos. En ciclos más avanzados, las condiciones son complejas e involucran el valor de una o más variables o registros.

El pseudocódigo es el siguiente:

      cx = N
    +-> ciclo:
    |  si cx = 0
    |   goto fin_ciclo
    | (Demás instrucciones del ciclo)  
    | decrementar cx  
    |_goto ciclo
    fin_ciclo:
    Instrucciones a ejecutar luego del ciclo


Implementación usando a cx como contador y realizando la comparación


En sintaxis AT&T: 

    movw $10, %cx /* Para el ejemplo, repetir 10 veces */
    ciclo:
    cmpw $0, %cx /* Comparar %cx con cero*/
    je fin_ciclo /* Si %cx = 0, ir a la etiqueta fin_ciclo */
    /* Demás instrucciones del ciclo */
    decw %cx /* %cx = %cx - 1 */
    jmp ciclo /* salto incondicional a la etiqueta ciclo*/
    fin_ciclo:
    /* Instrucciones a ejecutar después del ciclo */


En sintaxis Intel: 

    mov cx, 10 /* Para el ejemplo, repetir 10 veces */
    ciclo:
    cmp cx, 0 /* Comparar cx con cero*/
    je fin_ciclo /* Si cx = 0, ir a la etiqueta fin_ciclo */
    /* Demás instrucciones del ciclo */
    dec cx /* cx = cx - 1 */
    jmp ciclo /* salto incondicional a la etiqueta ciclo*/
    fin_ciclo:
    /* Instrucciones a ejecutar después del ciclo */


Implementación usando la instrucción loop y el registro cx


En sintaxis AT&T: 

      movw $10, %cx /* Para el ejemplo, repetir 10 veces */
      ciclo:
       /* Demás instrucciones dentro del ciclo 
       Importante: Recuerde que para el ciclo, se utiliza el registro %cx. 
        Por esta razón no es aconsejable utilizarlo dentro del ciclo. */
       
      loop ciclo /* Decrementar automáticamente %cx y verificar si es 
       mayor  que cero. Si %cx es mayor que cero, saltar 
       a la  etiqueta 'ciclo'. En caso contrario, 
       continuar la ejecución en la instrucción siguiente*/   

En sintaxis Intel: 

     mov cx, 10 /* Para el ejemplo, repetir 10 veces */  
     ciclo:
       /* Demás instrucciones dentro del ciclo 
       Importante: Recuerde que para el ciclo, se usa el registro cx. 
       Por esta razón no se debe utilizar dentro del ciclo.
       */   
    loop ciclo /* Decrementar automáticamente cx y verificar si es
     mayor que cero. Si cx es mayor que cero, saltar a la 
     etiqueta 'ciclo' En caso contrario, continuar la 
     ejecución en la instrucción siguiente*/

Ambas estrategias son válidas. Generalmente se utiliza la instrucción 'loop' y
el registro CX. Sin embargo, cuando la condición es más compleja, se utiliza la
primera aproximación.
