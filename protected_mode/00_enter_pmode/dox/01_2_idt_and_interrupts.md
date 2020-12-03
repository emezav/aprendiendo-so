IDT y gestión de interrupciones
==================================

Una interrupción es una señal que rompe con el flujo normal del procesador, y
que debe ser atendida inmediatamente.

Tipos de interrupción
-----------------------

En la arquitectura x86 se definen tres fuentes básicas de interrupción:
- Excepciones: Son condiciones de error que se presentan en la ejecución. Por
	ejemplo, al realizar una división por cero se lanza la excepción Division By
	Zero. Es una interrupción generada internamente por el procesador.
- Interrupciones de los dispositivos de hardware (discos, teclado, floppy, etc).
	Los dispositivos de hardware realizan solicitudes de interrupción (Interrupt
	Request - IRQ). Cada IRQ tiene asociado un número de interrupción predefinido,
	pero es posible cambiarlo por programación.
- Interrupciones por software, generadas mediante la instrucción int N donde N
	es el número de interrupción.

La arquitectura x86 soporta 256 interrupciones. De estas, las 32 primeras
(número 0 a 31) se asignan por defecto a las excepciones del procesador.

La siguiente tabla resume las diferentes interrupciones definidas en x86.

    | Int         | Descripción
    |-------------|-------------------------------------------------
    |  0  0x00    | Divide error:
    |             | Ocurre durante una instrucción DIV, cuando el divisor
    |             | es cero o cuando ocurre un desbordamiento del cociente.
    |             | Esta excepción no genera código de error.
    |-------------|--------------------------------------------------
    |  1  0x01    | (Reservada)
    |             | Esta excepción no genera código de error.  
    |
    |  2  0x02    | Nonmaskable interrupt:
    |             | Ocurre debido a una interrupcion de hardware que no se puede
    |             |  enmascarar.
    |             | Esta excepción no genera código de error.
    |
    |  3  0x03    | Breakpoint:
    |             | Ocurre cuando el procesador encuentra una instrucción INT 3
    |             | Esta excepción no genera código de error.
    |
    |  4  0x04    | Overflow:
    |             | Ocurre cuando el procesador encuentra una instrucción INTO
    |             | y el bit OF (Overflow) del registro EFLAGS se encuentra activo.
    |             | Esta excepción no genera código de error.
    |
    |  5  0x05    | Bounds check (BOUND instruction):
    |             | Ocurre cuando el procesador, mientras ejecuta una instrucción
    |             | BOUND, encuentra que el operando excede el límite especificado.
    |             | Esta excepción no genera código de error.
    |
    |  6  0x06    | Invalid opcode:
    |             | Ocurre cuando se detecta un código de operación inválido.
    |             | Esta excepción no genera código de error.
    |
    |  7  0x07    | Device Not Available (No Math Coprocessor)
    |             | Ocurre para alguna de las dos condiciones:
    |             | - El procesador encuentra una instrucción ESC (Escape)
    |             |   y el bit EM (emulate) bit de CR0 (control register zero)
    |             |   se encuentra activo.
    |             | - El procesador encuentra una instrucción WAIT o una instrucción
    |             |   ESC y los bits MP (monitor coprocessor) y TS (task switched)
    |             |   del registro CR0 se encuentran activos.
    |             | Esta excepción no genera código de error.
    |
    |  8  0x08    | Double fault:
    |             | Ocurre cuando el procesador detecta una excepción mientras trata
    |             | de invocar el manejador de una excepción anterior.
    |             | Esta excepción genera un código de error.
    |
    |  9  0x09    | Coprocessor segment overrun:
    |             | Ocurre cuando se detecta una violación de página o segmento
    |             | mientras se transfiere la porción media de un operando de 
    |             | coprocesador al NPX.
    |             | Esta excepción no genera código de error.
    |
    | 10  0xA     | Invalid TSS:
    |             | Ocurre si el TSS es inválido al tratar de cambiar de tarea (Task 
    |             |  switch).
    |             | Esta excepción genera código de error.
    |
    | 11  0xB     | Segment not present:
    |             | Ocurre cuando el procesador detecta que el bit P (presente) de un
    |             | descriptor de segmento es cero.
    |             | Esta excepción genera código de error.
    |
    | 12  0xC     | Stack exception:
    |             | Ocurre para las siguientes condiciones:
    |             | - Como resultado de una violación de límite en cualquier operación
    |             |   que se refiere al registro de segmento de pila (SS)
    |             | - Cuando se trata de establecer SS con un selector cuyo descriptor
    |             |   asociado se encuentra marcado como no presente, pero es válido
    |             | Esta excepción genera código de error.
    |
    | 13  0xD     | General protection violation (GP):
    |             | Cada violación de protección que no causa otra excepción
    |             | causa una GP.
    |             | - Exceder el límite de segmento para CS, DS, ES, FS, o GS
    |             | - Exceder el límite de segmento cuando se referencia una tabla de
    |             |   descriptores
    |             | - Transferir el control a un segmento que no es ejecutable
    |             | - Escribir en un segmento de datos de sólo lectura o en un 
    |             |   segmento de código
    |             | - Leer de un segmento marcado como sólo de ejecución
    |             | - Cargar en SS un selector que referencia a un segmento de sólo 
    |             |   lectura
    |             | - Cargar SS, DS, ES, FS, o GS con un selector que referencia a un
    |             |   descriptor de tipo "sistema"
    |             | - Cargar DS, ES, FS, o GS con un selector que referencia a un 
    |             |   descriptor de segmento marcado como ejecutable que además no se
    |             |   puede leer
    |             | - Cargar en SS un selector que referencia un descriptor  de 
    |             |   segmento ejecutable
    |             | - Acceder a la memoria por medio de DS, ES, FS, o GS cuando estos
    |             |   registros de segmento contienen un selector nulo
    |             | - Pasar la ejecución (task switch) a una tarea marcada como "Busy"
    |             | - Violar las reglas de privilegios
    |             | - Cargar CR0 con los bits PG=1 y PE=0 (habilitar la paginación y
    |             |      no habilitar el modo protegido)
    |             | - Lanzar una interrupción o una excepción a través de un trap 
    |             |   gate desde Modo Virtual 8086 a un privilegio (DPL) diferente 
    |             |   de cero 
    |             | Esta excepción genera código de error.
    |
    | 14  0xE     | Page fault:
    |             | Ocurre cuando la paginación está habilitada (PG = 1) en CR0
    |             | y el procesador detecta alguna de las siguientes condiciones
    |             | cuando trata de traducir una dirección lineal a física:
    |             | - El directorio de tabla de páginas o la tabla de páginas requerido
    |             |   para realizar la traducción tiene 0 en su bit de presente (P)
    |             | - El procedimiento actual no tiene los suficientes privilegios para
    |             |   acceder la página indicada.
    |             | Esta excepción genera código de error.
    |
    | 15  0xF     | (Reservada)
    |             | Esta excepción no genera código de error.
    |
    | 16  0x10    | x87 FPU Floating-Point Error (Math Fault)
    |             | Ocurre cuando el procesador detecta una señal del coprocesador en el
    |             | pin de entrada ERROR#.
    |
    | 17  0x11    | Alignment Check
    |             | Ocurre cuando se realiza una referencia de datos en la memoria a 
    |             | una región no alineada.
    |             | Esta excepción genera código de error.
    | 
    | 18  0x12    | Machine Check
    |             | Depende del modelo y las características del procesador.
    |             | Esta excepción no genera código de error.
    |          
    | 19  0x23    | SIMD Floating-Point Exception
    |             | Ocurre cuando existe un error en las instrucciones SSE/SSE2/SSE3.
    |             | Esta excepción no genera código de error.
    |          
    |
    | 20  0x24    | Reservadas por Intel.
    |  hasta      |
    | 31  0x1F    | Estas excepciones no generan código de error.
    |             |
    |             
    |              
    | 32  0x20    | Interrupción externa o interrupción invocada mediante la
    |  hasta      | instrucción INT N
    | 255 0xFF    | Estas interrupciones no generan código de error


Tabla de Descriptores de Interrupción (IDT)
-------------------------------------------

La IDT es una estructura de datos que usa el procesador en el momento en que
ocurre la interrupción, y que debe estar configurada antes de habilitar las
interrupciones. Es una tabla que contiene una serie de entradas denominadas
"descriptores", que definen entre otros parámetros la dirección de memoria en la
cual se encuentra cada rutina de manejo de interrupción.

El siguiente esquema muestra la IDT y las rutinas de manejo de interrupción en
memoria:

        +------------------------------+
        |                              |
        |                              |
        +------------------------------+
        | Rutina de manejo de          |<------+
        | interrupción 255             |       |
        +------------------------------+       |
        |                              |       |
        | ...                          |       |
        +------------------------------+       |
        | Rutina de manejo de          |       |
        | interrupción 2               |       |
        +------------------------------+       |
        | Rutina de manejo de          |<----+ |
        | interrupción 1               |     | |
        +------------------------------+     | |
        | Rutina de manejo de          | <-+ | |
        | interrupción 0               |   | | |
        +------------------------------+   | | |
        |                              |   | | |
        |                              |   | | |
        |                              |   | | |
        +------------------------------+   | | |
        | Descriptor de Interrupción   |<------+
        | 255                          |   | |  
        +------------------------------+   | |  
        | ...                          |   | |  
        |                              |   | |
        +------------------------------+   | |
        | Descriptor de Interrupción   |<----+
        | 1                            |   |
        +------------------------------+   |
        | Descriptor de Interrupción   |<--+
        | 0                            |
        +------------------------------+ <--- Inicio de la Tabla de Descriptores
        |                              |      de Interrupción (IDT)
        |                              |
        |                              |
        |                              |
        |                              |
        +------------------------------+         

La IDT está conformada por 256 descriptores, uno para cada interrupción. Cada
descriptor ocupa 8 bytes, y puede ser de uno de los siguientes tipos:
- Task Gate: Permite invocar una tarea. Se usa cuando se implementa multi tarea
	por hardware.
- Interrupt Gate: Permite invocar una rutina de servicio de interrupción.
	Al ejecutar una Interrupt Gate, se deshabilitan automáticamente las
	interrupciones.
- Trap Gate: Al igual que una Interrupt Gate, permite invocar una rutina de
	servicio de interrupción. Sin embargo, a diferencia de una Interrupt Gate,
	las interrupiciones quedan habilitadas.

Cada entrada tiene el siguiente formato:

     31                23               15--FLAGS---------- 7                0
     +-----------------------------------+-+-----+---------+-----+---------+
     |                                   | |     | TYPE    |  (ALWAYS 0)   |
     |           OFFSET 31..16           |P| DPL |0 D 1 1 0|0 0 0 0 0 0 0 0| 4
     |                                   | |     |         |               |
     +-----------------------------------+-+-----+---------+---------------+
     |                                   |                                 |
     |            SELECTOR               |          OFFSET 15..0           | 0
     |                                   |                                 |
     +-----------------------------------+---------------------------------+

En donde:
- Offset: Desplazamiento (offset) en el cual se encuentra la rutina de manejo de
	interrupción (la dirección de memoria de la rutina) dentro de un segmento de
	código.
- Selector: Selector que referencia al descriptor de segmento de código en la
	GDT dentro del cual se encuentra la rutina de manejo de interrupción.
- D : Tipo de descriptor : (0=16 bits), (1=32 bits)
- FLAGS : compuesto por los bits P (1 bit), DPL (2 bits) y TYPE (5 bits). Para
	un interrupt gate, el valor de FLAGS es 0x8E = 10001110 (P=1, DPL=0, D=1)

La dirección lógica segmento : offset que se obtiene del descriptor se traduce a
una dirección lineal usando la GDT o la IDT. Si la paginación se encuentra
deshabilitada (por defecto), la dirección lineal es la misma dirección física en
la cual se encuentra la rutina que atenderá la interrupción. En el caso
contrario, a partir de la dirección lineal se obtendrá la dirección física de la
rutina de manejo de interrupción.

Carga de la IDT
---------------
Para cargar la IDT se utiliza la instrucción de ensamblador 

lidt ptr_addr

La instrucción idt toma el puntero y lo carga en el registro IDTR del
procesador. ptr_addr corresponde a la dirección de memoria en la cual se
encuentra una estructura de datos que describe la IDT. Esta estructura de datos
se denomina 'puntero a la IDT', 'IDT Pointer'.

El puntero al IDT tiene el siguiente formato: 

    47                  15              0
     +----------------------------------+
     |      base         |    límite    |
     +----------------------------------+

en donde: 
- base : dirección lineal de la IDT, que corresponde a la dirección de memoria
	de la IDT.
- límite : tamaño de la IDT en Bytes. Si la IDT tiene 256 entradas y el tamaño
	de cada entrada es de 8 bytes, el tamaño total de la IDT es de 2048 bytes (2
	KB).


Gestión de Interrupciones
-------------------------

Cuando ocurre la interrupción N, el procesador busca la entrada N dentro de la
IDT, y de ella obtiene la dirección lineal de la rutina de manejo de la
interrupción N a la cual se debe pasar el control.

Antes de pasar el control a la rutina de manejo de interrupción, el procesador
almacena en el tope de la pila el valor de EFLAGS, CS y EIP en este orden. Esto
permitirá ejecutar la rutina de manejo de interrupción y luego continuar la
ejecución en el punto exacto en el que se interrumpió al procesador.

Si la interrupción genera un código de error (algunas de las excepciones de
x86), este código de error también se inserta en la pila.

A continuación se ilustra el estado de la pila en el momento de pasar el control
a la rutina de manejo de interrupción.

Interrupción sin código de error
--------------------------------

Cuando ocurre una interrupción que no tiene código de error, la pila se
encontrará en el siguiente estado en el momento  de pasar el control a la rutina
de manejo de interrupción:

      +--------------------+    
      |  valor             |                                       
      +--------------------+ <- SS: ESP antes de ocurrir la interrupción
      | EFLAGS             |    
      +--------------------+     
      | CS                 |     
      +--------------------+
      | EIP                |  
      +--------------------+ <- SS: ESP al momento de pasar el control a la 
      | Espacio            |   rutina de manejo de interrupción
      | disponible en la   |
      | pila               |  
      |                    |
      |                    |
      +--------------------+ <---  Base de la pila
      |                    |
      |                    |
      +--------------------+

Si el segmento de código en el cual se encuentra la rutina de manejo de
interrupción tiene un nivel de privilegios (DPL) diferente al nivel de
privilegios del segmento de código en el que se encuentra el código
interrumpido, ocurre un _Cambio de Contexto_ de pila. El procesador almacena
primero SS y ESP, y luego almacena EFLAGS, CS e IP en la pila.

    +--------------------------+
    | valor en la pila         | Pila del código (o tarea) antes de ocurrir la
    |--------------------------| interrupción o excepción                  
    | valor en la pila         |            
    +--------------------------+  <-- OLD SS: OLD ESP -----+
                                                           |
                                                           |
    +------------------------------------------------------+
    |  +--------------------------+
    |  | old ss                   | Estos valores son almacenados automáticamente
    |  |--------------------------| en la pila cuando ocurre una interrupción
    +--|--> old esp               | o excepción y hay cambio de privilegios
       |--------------------------| ..
       | eflags                   | ..
       |--------------------------| ..
       | old cs                   | ..
       |--------------------------| ..
       | old eip                  | ..
       +--------------------------+  <-- ESP

Interrupción con código de error
------------------------------------
Cuando ocurre una interrupción que tiene código de error asociado, la pila se
encontrará en el siguiente estado antes de pasar el control a la rutina de
manejo de interrupción:

      +--------------------+    
      |  valor             |                                       
      +--------------------+ <- SS: ESP antes de ocurrir la interrupción
      | EFLAGS             |    
      +--------------------+     
      | CS                 |     
      +--------------------+
      | EIP                |     
      +--------------------+
      | Código de error    |  
      +--------------------+ <- SS: ESP al momento de pasar el control a la 
      | Espacio            |        rutina de manejo de interrupción
      | disponible en la   |
      | pila               |  
      |                    |
      |                    |
      +--------------------+ <---  Base de la pila
      |                    |
      |                    |
      +--------------------+

Si el segmento de código en el cual se encuentra la rutina de manejo de
interrupción tiene un nivel de privilegios (DPL) diferente al nivel de
privilegios del segmento de código en el que se encuentra el código
interrumpido, también ocurre un _Cambio de Contexto_ de pila. El procesador
almacena primero SS y ESP, y luego almacena EFLAGS, CS e IP en la pila.

    +--------------------------+
    | valor en la pila         | Pila del código (o tarea) antes de ocurrir la
    |--------------------------| interrupción o excepción                  
    | valor en la pila         |            
    +--------------------------+  <-- OLD SS: OLD ESP -----+
                                                           |
                                                           |
    +------------------------------------------------------+
    |  +--------------------------+
    |  | old ss                   | Estos valores son almacenados automáticamente
    |  |--------------------------| en la pila cuando ocurre una interrupción
    +--|--> old esp               | o excepción y hay cambio de privilegios
       |--------------------------| ..
       | eflags                   | ..
       |--------------------------| ..
       | old cs                   | ..
       |--------------------------| ..
       | old eip                  | ..
       |--------------------------| ..
       | Código de error          | ..
       +--------------------------+  <-- ESP


