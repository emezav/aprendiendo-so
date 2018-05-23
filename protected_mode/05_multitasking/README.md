Multi - tarea
================================
Este proyecto implementa el código para crear y ejecutar múltiples tareas
de kernel.

El Sistema Operativo deberá inicializar un entorno en el cual las tareas se
puedan ejecutar. Este entorno está constituido por:

1. Un Segmento de Estado de Tarea (TSS) referenciado por un Descriptor dentro de
   la GDT y por su respectivo selector.
2. Dos descriptores Descriptores de segmento de código y datos dentro de la GDT
   par la tarea y su respectivos  selectores, que describen la organización en
   memoria de la tarea.
3. Un mecanismo para crear una tarea.
4. Un mecanismo para pasar el control de la CPU a las tareas.
5. Un mecanismo para retornar el control de la CPU al sistema operativo.
6. Un mecanismo de comunicación entre las tareas y el sistema operativo.

A este entorno se suman los descriptores de código y datos del kernel,
necesarios para operar en modo protegido de 32 bits.

Consulte el código de los archivos kernel/include/tss.h,
kernel/include/task.h, kernel/src/tss.c y kernel/src/task.c.

Estructura de Datos de una Tarea
--------------------------------

El sistema operativo debe almacenar la información de cada tarea en una
estructura de datos que permita identificarla de forma única. La información de
esta estructura de datos depende de cada sistema operativo, pero se pueden
identificar algunos campos relevantes:

- Identificador de la Tarea (generalmente denominado PID - Process ID)
- Identificador de la Tarea Padre (PPID), si se usa en esquema jerárquico de tareas.
- Estado de la tarea, desde el punto de vista del sistema operativo. Algunos
  estados generales son:
  - Listo : La tarea se encuentra en espera de que se le asigne la CPU
  - Ejecución : La tarea tiene asignada la CPU y se están ejecutando sus instrucciones
  - Bloqueado: La tarea se encuentra esperando a que suceda un evento para continuar su ejecución
  - Terminado : La tarea ha terminado la ejecución y está lista para que el sistema operativo libere los recursos asociados a ella
- Nivel de privilegios de la tarea, que puede estar asociado a los niveles de privilegios del procesador o a una jerarquía de privilegios definida por el sistema operativo
- Dirección relativa a la sección de código de la tarea en la cual se debe comenzar su ejecución
- Dirección de inicio de la región de memoria asignada a la tarea (base)
- Tamaño de la tarea (Límite)

Cada sistema puede incluir estos y otros campos para permitir la gestión de la
tarea. También se puede incluir dentro de la tarea el estado de los registros
del procesador. A la información que el sistema operativo almacena se le
denomina __Bloque de control de proceso__, BCP.

Segmento de Estado de la Tarea
------------------------------

Se debe crear un Segmento de Tarea (Task Segment). Este segmento es una
estructura de datos que almacena la información del estado de una tarea. Se
deberá almacenar un descriptor dentro de la GDT, que defina la posición de
memoria del TSS, su tamaño y su nivel de privilegios.

                 Espacio Lineal                                 GDT
                                                    +--------------------------+
          4 GB                                      |                          |
            +------------------------+              |                          |
            |                        |              +--------------------------+    
            |                        |              |                          |  
            |                        |              |                          |   
            |                        |              +--------------------------+   
            |                        |              |                          |   
            |                        |              |                          |   
            |                        |              +--------------------------+   
            |                        |              | Descriptor de TSS  DPL=0 |
            |                        |   +----------| Base = TSS_Loc           |
            |                        |   |          +--------------------------+
            |                        |   |          | Datos del Kernel   DPL=0 |
            |                        |   |      +---| Base = 0, Límite = 4 GB  |
            |                        |   |      |   +--------------------------+
            |                        |   |      |   | Código del Kernel  DPL=0 |
            +------------------------+   |      +---| Base = 0, Límite = 4GB   |
            |                        |   |      |   +--------------------------+
            | Sistema Operativo      |   |      |   |   Descriptor Nulo        |
            |                        |   |      |   |                          |
            +------------------------+   |      |   +--------------------------+
            |    TSS                 |   |      | 
    TSS_Loc +------------------------+ <-+      |  
            |                        |          |  
            |                        |          |  
            |                        |          |  
          0 +------------------------+ <--------+   

Selectores
----------
Para el descriptor de segmento de código del kernel, el selector correspondiente
es:

     15           3 2 1 0 
     +-------------+-+--+
     |0000000000001|0|00|   Selector de código = 1000 = 0x08
     +-------------+-+--+   Índice = 1, TI = 0 (GDT), DPL = 00


Para el descriptor de segmento de datos del kernel, el selector correspondiente
es:

    15           3 2 1 0 
    +-------------+-+--+
    |0000000000010|0|00|   Selector de código = 10000 = 0x10
    +-------------+-+--+   Índice = 2, TI = 0 (GDT), DPL = 00


Para el descriptor de TSS, el selector correspondiente es:

    15           3 2 1 0 
    +-------------+-+--+
    |0000000000011|0|00|   Selector de código = 11000 = 0x18
    +-------------+-+--+   Índice = 3, TI = 0 (GDT), DPL = 00

Descriptores de Segmento de Código y Datos para la Tarea
---------------------------------------------------------
Los descriptores de código y datos de la tarea se diferencian de los
descriptores de código y datos del kernel en su nivel de privilegios, y en su
dirección base y límite. Debido a que se desea establecer un entorno que brinde
algún grado de protección entre las tareas, los descriptores sólo deberán
permitir que las tareas tengan acceso a su región de memoria. Esto se ilustra en
el siguiente esquema: 

           Espacio Lineal                                    GDT
                                                    +--------------------------+
          4 GB                                      |                          |
            +------------------------+              |                          |
            |                        |              +--------------------------+
            |                        |              | Datos de la Tarea  DPL=3 |
            |                        |       +------| Base = Addr, Límite = N  |
            |                        |       |      +--------------------------+
            |                        |       |      | Código de la Tarea DPL=3 |
            |                        |       +------| Base = Addr, Límite = N  |
            |                        |       |      +--------------------------+
          - +------------------------+       |      | Descriptor de TSS  DPL=0 |
          | | Espacio de memoria     |   +----------| Base = TSS_Loc           |
          | | para la tarea          |   |   |      +--------------------------+
          | |                        |   |   |      | Datos del Kernel   DPL=0 |
        N | |                        |   |   |  +---| Base = 0, Límite = 4 GB  |
          | |                        |   |   |  |   +--------------------------+
          | |                        |   |   |  |   | Código del Kernel  DPL=0 |
     Addr - +------------------------+ <-|---+  +---| Base = 0, Límite = 4GB   |
            |                        |   |      |   +--------------------------+
            | Sistema Operativo      |   |      |   |   Descriptor Nulo        |
            |                        |   |      |   |                          |
            +------------------------+   |      |   +--------------------------+
            |    TSS                 |   |      | 
    TSS_Loc +------------------------+ <-+      |  
            |                        |          | 
            |                        |          | 
            |                        |          | 
          0 +------------------------+ <--------+   

Para el diagrama anterior, los selectores correspondientes son:
Para el descriptor de segmento de código del kernel, el selector correspondiente
es:

    15           3 2 1 0 
    +-------------+-+--+
    |0000000000001|0|00|   Selector de código = 1000 = 0x08
    +-------------+-+--+   Índice = 1, TI = 0 (GDT), DPL = 0x00


Para el descriptor de segmento de datos del kernel, el selector correspondiente
es:

    15           3 2 1 0 
    +-------------+-+--+
    |0000000000010|0|00|   Selector de código = 10000 = 0x10
    +-------------+-+--+   Índice = 2, TI = 0 (GDT), DPL = 0x00

Para el descriptor de TSS, el selector correspondiente es:

    15           3 2 1 0 
    +-------------+-+--+
    |0000000000011|0|00|   Selector de datos = 11000 = 0x18
    +-------------+-+--+   Índice = 3, TI = 0 (GDT), DPL = 0x00


Para el descriptor de segmento de código de la tarea, el selector
correspondiente es:•

    15           3 2 1 0 
    +-------------+-+--+
    |0000000000100|0|11|   Selector de código = 100011 = 0x23
    +-------------+-+--+   Índice = 4, TI = 0 (GDT), DPL = 0x11


Para el descriptor de segmento de datos de la tarea, el selector correspondiente
es:

    15           3 2 1 0 
    +-------------+-+--+
    |0000000000101|0|11|   Selector de datos = 101011 = 0x2b
    +-------------+-+--+   Índice = 5, TI = 0 (GDT), DPL = 0x11


Mecanismo para pasar el control de la CPU a las tareas
------------------------------------------------------
Una vez que el sistema operativo ha cargado y organizado la tarea en memoria, se
debe establecer un entorno inicial de ejecución. Este entorno hace parecer que
la tarea anteriormente fue interrumpida, y va a continuar ejecutándose.

Se debe recordar que el estado del procesador cuando se interrumpe la tarea se
almacena en la pila, por lo tanto se debe configurar un marco de pila con igual
configuración similar a la que se presenta en el siguientes esquema:

     +--------------------------+
     | old SS                   | Selector de datos de la tarea
     |--------------------------|
     | old ESP                  | Tope de la pila para la tarea
     |--------------------------|
     | EFLAGS                   | EFLAGS para la tarea, con el flag IF (Interrupt
     |--------------------------| Flag) habilitado
     | old CS                   | Selector de código de la tarea
     |--------------------------| 
     | old EIP                  | Dirección lineal de la rutina main()
     |--------------------------|
     | 0 (código de error)      | 0
     |--------------------------|
     | # de excepción generada  | 0
     |--------------------------|
     | EAX                      | 0
     |--------------------------|
     | ECX                      | 0
     |--------------------------|
     | EDX                      | 0
     |--------------------------|
     | EBX                      | 0
     |--------------------------|
     | ESP antes de pusha       | 0
     |--------------------------|
     | EBP                      | 0
     |--------------------------|
     | ESI                      | 0
     |--------------------------|
     | EDI                      | 0
     |--------------------------|
     | DS                       | Selector de datos de la tarea
     |--------------------------|
     | ES                       | Selector de datos de la tarea
     |--------------------------|
     | FS                       | Selector de datos de la tarea
     |--------------------------|
     | GS                       | Selector de datos de la tarea
     |--------------------------|<-- ESP de la tarea
    

Luego el sistema debe simular el _retorno de interrupción_. Este retorno de
interrupción obtiene de la pila el estado del procesador (el estado de sus
registros), con lo cual se establece el entorno de ejecución de la tarea
compuesto por los selectores que apuntan a los descriptores de segmento de la
tarea definidos en la GDT, el valor de IP, EFLAGS y de sus registros de
propósito general.

Mecanismo para pasar el control de la CPU al sistema operativo
-----------------------------------------------------------------

Cuando una tarea recibe el control del sistema operativo, existen dos
estrategias para retornar el control de la CPU:

- Invocar una interrupción por software (Instrucción INT de ensamblador), con lo
	cual se almacena en la pila el estado del procesador y se pasa el control a
	la rutina de manejo de interrupción correspondiente (Sistema Operativo).
- Cuando ocurre una interrupción periódica del Timer, se almacena el estado del
	procesador (para la tarea) y se pasa el control a la rutina de manejo de
	interrupción del Timer.

De esta forma, dentro de las rutinas de manejo de interrupción se puede
adicionar la lógica necesaria para determinar si se retorna el control de la CPU
a la tarea que se interrumpió, si se debe terminar la tarea o pasar el control
de la CPU a otra tarea.



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
proyecto para estudiar el proceso que realiza la utilidad *make* para 
compilar el código fuente y generar la imagen de disco.

Depuración paso a paso
======================
En el archivo bochsrc.txt se ha activado el *Magic break*, por lo cual si se
incluye la instrucción *xchg bx, bx* en cualquier parte del código, se pausará
la ejecución cuando se usa el emulador bochs con el depurador gráfico activado
(comando *make bochsdbg*).


Vea también
===========
- Intel® 64 and IA-32 Architectures Software Developer’s Manual Volume 3A:
  System Programming Guide, Part 1 https://software.intel.com/en-us/articles/intel-sdm
