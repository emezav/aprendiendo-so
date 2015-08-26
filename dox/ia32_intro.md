Generalidades de la arquitectura IA-32
======================================

Intel, el fabricante de los procesadores de arquitectura IA-32, ha
decidido mantener compatibilidad hacia atrás para permitir que el código
desarrollado para procesadores desde 386 o 486 pueda ser ejecutado en
procesadores actuales. Esto implica una serie de decisiones de diseño en
la estructura interna y en el funcionamiento de los procesadores, que en
ciertas ocasiones limita a los programas pero que también ofrece una
ventaja competitiva relacionada con la adopción masiva de los
procesadores y la posibilidad de ejecutar programas creados para
procesadores anteriores en las versiones actuales sin virtualmente
ninguna modificación.

Cada procesador actual cuenta con algunas o todas las características de
la arquitectura IA-32. Por ejemplo, algunos procesadores actuales poseen
múltiples núcleos con registros de 32 bits o múltiples núcleos con
registros de 64 bits. Un procesador Intel Core 2 Duo cuenta con dos
núcleos con registros de 32 bits, y un procesador Xeon generalmente
incluye varios núcleos con registros de 64 bits. La generación actual de
procesadores Intel Core (Core I3, Core I5 y Core I7) implementan
arquitecturas de 2, 4 y hasta 6 núcleos con registros de 64 bits.

No obstante, para mantener la compatibilidad hacia atrás, todos los
procesadores inician en un modo de operación denominado Modo Real
(también llamado Modo de Direcciones Real o Real Addres Mode), en el
cual se comportan como un procesador 8086 con algunas extensiones que le
permiten habilitar el modo de operación en el cual aprovechan todas sus
características.

El conocimiento de la arquitectura IA-32 ofrece una posibilidad sin
igual para el aprendizaje de la programación básica de una amplia gama
de procesadores, desde la programación en modo real hasta la
programación en modo protegido, usado por los Sistemas Operativos
Modernos.

En los siguientes apartados se presentan los conceptos básicos
relacionados con la programación de procesadores de la Arquitectura
IA-32

Modos de Operación
==================

Los procesadores IA-32 pueden operar en varios modos:

- Modo protegido: Este es el modo nativo del procesador. Aprovecha todas
  las características de su arquitectura, tales como registros de 32 bits,
  y el acceso a todo su conjunto de instrucciones y extensiones.

- Modo real: En este modo el procesador se encuentra en un entorno de
  ejecución en el cual se comporta como un 8086 muy rápido, y sólo tiene
  acceso a un conjunto limitado de instrucciones que le permiten ejecutar
  tareas básicas y habilitar el modo protegido. La limitación más notable
  en este modo consiste en que sólo se puede acceder a los 16 bits menos
  significativos de los registros de propósito general, y sólo se pueden
  utilizar los 20 bits menos significativos del bus de direcciones. Esto
  causa que en modo real solo se pueda acceder a 1 Megabyte de memoria.
  Todos los procesadores de IA-32 inician su operación en este modo.

- Modo de mantenimiento del sistema: En este modo se puede pasar a un
  entorno de ejecución limitado, para realizar tareas de mantenimiento o
  depuración.

- Modo Virtual 8086: Este es un sub-modo al cual se puede acceder cuando
  el procesador opera en modo protegido. Permite ejecutar código
  desarrollado para 8086 en un entorno multi-tarea y protegido.

- Modo IA32-e: Para procesadores de 64 bits, además de los modos
  anteriores existen otros dos sub-modos: modo de compatibilidad y modo de
  64 bits. El modo de compatibilidad permite la ejecución de programas
  desarrollados para modo protegido sin ninguna modificación, y el modo de
  64 bits proporciona soporte para acceder a los 64 bits de los registros
  y un espacio de direcciones mayor que 64 Gigabytes.
  
Entorno de Ejecución
=====================

Cualquier programa o tarea a ser ejecutado en un procesador de
arquitectura IA-32 cuenta con un entorno de ejecución compuesto por un
espacio de direcciones de memoria y un conjunto de registros. A
continuación se describen estos componentes.

Espacio Lineal de Direcciones
-----------------------------

En la arquitectura IA-32 la memoria puede ser vista como una secuencia
lineal (o un arreglo) de bytes, uno tras del otro. A cada byte le
corresponde una dirección única (Ver figura).

                 Espacio Lineal de Direcciones
                +------------------------------+
                |                              |
                |                              |
                |                              |
                |                              |
                |                              |
                |                              |
                |                              |
                |                              |
                +------------------------------+
                | Valor (byte)                 |<-- Siguiente dirección lineal
                +------------------------------+
                | Valor (byte)                 |
                +------------------------------+<--+
                |                              |   | Dirección lineal
                |                              |   | (desplazamiento desde el
                |                              |   | inicio del espacio lineal)
                |                              |   |
                |                              |   |
                +------------------------------+   |

El código dentro de una tarea o un programa puede referenciar un espacio
lineal de direcciones tan grande como lo permitan los registros del
procesador. Por ejemplo, en modo real sólo es posible acceder a los 64
KB dentro de un segmento definido (2 elevado a la 16 = 64 KB), y en modo
protegido de 32 bits se puede acceder a un espacio lineal de hasta 4 GB
(2 elevado a la 32 = 4 GB).

Este espacio lineal puede estar mapeado directamente a la memoria
física. Si el procesador cuenta con las extensiones requeridas, es
posible acceder a un espacio físico de hasta 64 Gigabytes.

Se debe recordar que la arquitectura IA-32 siempre hace uso de un modelo
de memoria segmentado, sin importar su modo de operación. Los
sistemas operativos actuales optan por usar un modelo plano (Flat) en
modo protegido, por lo cual pueden tener acceso a todo el espacio lineal
de direcciones.

Espacio de Direcciones de Entrada / Salida
------------------------------------------

Los procesadores IA-32 incluyen otro espacio, diferente
al espacio lineal de direcciones , llamado espacio de direcciones de
Entrada / Salida. A este espacio de 65536 (64K) direcciones se mapean
los registros de los controladores de dispositivos de entrada / salida
como el teclado, los discos o el mouse (Ver figura).

                 Espacio de direcciones de E/S
                 
                       65535           
    +----------------+
    |                |
    |                |               Controlador de           Dispositivo
    |                |               Dispositivo             (disco, teclado, etc)
    +----------------+               +---------+              +---------------+
    |   byte         |<------------- | estado  |<-------------|               |
    +----------------+               +---------+              |               |
    |   byte         |<------------- | control |<-------------|               |
    +----------------+               +---------+              +---------------+
    |                |
    |                |
    +----------------+

El acceso al espacio de direcciones de E/S se realiza a través de un par
de instrucciones específicas del procesador (in y out). Al leer o
escribir un byte en una dirección de E/S, el byte se transfiere al
puerto correspondiente del dispositivo.

Se debe consultar la documentación de cada dispositivo de E/S para
determinar cuales son las direcciones de E/S a través de las cuales se
puede acceder a los registros de su controlador.

Por ejemplo, el controlador de teclado (8042) tiene asignadas las
siguientes direcciones de entrada / salida:

Dirección de E/S | Operación | Descripción
-----------------|-----------|-------------
0x60             | Lectura   | Buffer de entrada
0x60             | Escritura | Puerto de comandos
0x64             | Lectura   | Registro de Estado del teclado
0x64             | Escritura | Puerto de comandos

Este controlador deberá ser programado para habilitar la línea de
direcciones A20, que en los procesadores actuales se encuentra
deshabilitada al inicio para permitir la compatibilidad con programas
desarrollados para procesadores anteriores.

Conjunto de Registros IA-32
---------------------------

El procesador cuenta con una serie de registros en los cuales puede
almacenar datos. Estos registros pueden ser clasificados en:

- Registros de propósito general: Utilizados para almacenar valores,
  realizar operaciones aritméticas o lógicas o para referenciar el espacio
  de direcciones lineal o de E/S. En procesadores IA-32 bits existen ocho
  (8) registros de propósito general, cada uno de los cuales tiene un
  tamaño de 32 bits. Estos registros son: EAX, EBX, ECX, EDX, ESI, EDI,
  ESP y EBP. A pesar que se denominan registros de propósito general, y
  pueden ser utilizados como tal, estos registros tienen usos especiales
  para algunas instrucciones del procesador. Ppor ejemplo la instrucción
  DIV (dividir) hace uso especial de los registros EAX y EDX, dependiendo
  del tamaño del operando.

- Registros de segmento: Estos registros permiten almacenar apuntadores al
  espacio de direcciones lineal. Los procesadores IA-32 poseen seis (6)
  registros de segmento. Estos son: CS (código), DS (datos), ES, FS, GS
  (datos), y SS (pila). Su uso depende del modo de operación. En modo
  real, los registros de segmento almacenan un apuntador a la dirección
  lineal del inicio del segmento dividida en 16. En modo protegido se
  denominan selectores, y contienen un apuntador a una estructura de datos
  en la cual se describe un segmento de memoria.

- Registro EFLAGS: Este registro de 32 bits contiene una serie de banderas
  (flags) que tienen diversos usos. Algunas reflejan el estado del
  procesador y otras controlan su ejecución. Existen instrucciones
  específicas para modificar el valor de EFLAGS. Otras instrucciones
  modifican el valor de EFLAGS de forma implícita. Por ejemplo, si al
  realizar una operación aritmética o lógica se obtiene como resultado
  cero, el bit ZF (Zero Flag) del registro EFLAGS se establece en 1, para
  indicar esta condición. Esta bandera puede ser chequeada para realizar
  algún tipo de opración o salto dentro del código.

- Registro EIP: Este registro almacena el apuntador a la dirección lineal
  de la siguiente instrucción que el procesador debe ejecutar. Esta
  dirección es relativa al segmento al cual se referencia con el registro
  de segmento CS.

- Registros de control: El procesador posee cinco (5) registros de control
  CR0 a CR5. Estos registros junto con EFLAGS controlan la ejecución del
  procesador.

- Registros para el control de la memoria: Estos registros apuntan a las
  estructura de datos requeridas para el funcionamiento del procesador en
  modo protegido. Ellos son: GDTR, IDTR, TR y LDTR.

- Registros de depuración: Estos registros contienen información que puede
  ser usada para depurar el código que está ejecutando el procesador. Los
  procesadores IA-32 cuentan con ocho (8) registros de depuración, DR0 a
  DR7.

- Registros específicos: Cada variante de procesador IA-32 incluye otros
  registros, tales como los registros MMX, los registros de la unidad de
  punto flotante (FPU) entre otros.

Algunos registros de propósito general pueden ser sub-divididos en
registros más pequeños a los cuales se puede tener acceso. Esto permite
la compatibilidad con programas diseñados para procesadores anteriores.

A continuación se presentan las posibles sub-divisiones de los registros
de propósito general, considerando procesadores de hasta 64 bits:


64 bits | 32 bits | 16 bits | 8 bits | 8 bits
--------|---------|---------|--------|-------
RAX     |    EAX  |   AX    |   AH   |    AL
RBX     |    EBX  |   BX    |   BH   |    BL
RCX     |    ECX  |   CX    |   CH   |    CL
RDX     |    EDX  |   DX    |   DH   |    DL


64 bits | 32 bits | 16 bits
--------|---------|--------- 
RSI     |    ESI  |   SI    
RDI     |    EDI  |   DI    
RSP     |    ESP  |   SP    
RBP     |    EBP  |   BP    

A nivel de programación, es posible acceder a cada uno de estos
sub-registros de acuerdo con el modo de operación. Por ejemplo, para
modo de direcciones real, es posible usar los registros de 8 bits y los
registros de 16 bits. En modo protegido se puede usar los registros de
8, 16 y 32 bits. Si el procesador cuenta con registros de 64 bits y se
encuentra en el modo de 64 bits, es posible acceder a los registros de
8, 16, 32 y 64 bits.

La siguiente figura muestra como se encuentran dispuestos los bits de
los registros de propósito general. Los registros EBX, ECX y EDX se
encuentran dispuestos de la misma forma que EAX. Los registros EDI, ESP
Y EBP se disponen de la misma forma que ESI.

               Subdivisión de los registros EAX, EBX, ECX y EDX
    63                                   31                15         7       0
    +------------------------------------+-----------------+---------+--------+
    |                                    |                 |         |        |
    |                                    |                 |         |        |
    +------------------------------------+-----------------+---------+--------+
                                                           |-- AH ---|-- Al --|     
                                                           |------- AX -------|
                                         |---------------- EAX ---------------| 
    |-------------------------------   RAX  ----------------------------------|                     


               Subdivisión de los registros ESI, EDI, ESP y EBP
    63                                   31                15                 0
    +------------------------------------+-----------------+------------------+
    |                                    |                 |                  |
    |                                    |                 |                  |
    +------------------------------------+-----------------+------------------+
                                                           |------- SI -------|
                                         |---------------- ESI ---------------| 
    |-------------------------------   RSI  ----------------------------------|                     

Formato de almacenamiento de datos
----------------------------------

El formato de almacenamiento de la arquitectura IA-32 es Little-Endian,
lo cual significa que los bits menos significativos de un número se
almacenan en las posiciones menores de la memoria y de los registros, y
los bits más significativos se almacenan en posiciones superiores. La
siguiente figura muestra cómo se almacena el número 0x7C00 en un
registro y en la memoria.

El número 0x7C00 almacenado en un registro se ve así:

    Número en formato hexadecimal: 0x7C00
                             
                  Número almacenado en un registro
             Bit más                                      Bit menos
             significativo                              significativo
                15                                              0
    +-----------------------------------------------------------+
    | 0| 0| 0| 0| 0| 1| 1| 1| 1| 1| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 
    +-----------------------------------------------------------+
                                                    |--- 0 -----|
                                        |--- 0 -----|
                            |--- C -----|
                 |--- 7 ----|


El mismo número almacenado en memoria se ve así:

               Número almacenado en memoria
               Cada dirección de memoria almacena un byte (8 bits)
               
               +-----------------------+
               |  |  |  |  |  |  |  |  |                   ^
               +-----------------------+                   |  Dirección de 
               |  |  |  |  |  |  |  |  |                   |  incremento
               +-----------------------+                   |  en la memoria
               |  |  |  |  |  |  |  |  |                   |
               +-----------------------+
               |  |  |  |  |  |  |  |  |
               +-----------------------+
       = 0x7C  | 0| 1| 1| 1| 1| 1| 0| 0|   <-- Dirección mayor de memoria
               +-----------------------+
       = 0x00  | 0| 0| 0| 0| 0| 0| 0| 0|   <-- Dirección menor de memoria
               +-----------------------+           


Registros principales de IA-32
------------------------------

Si bien todos los registros de la arquitectura IA-32 son importantes,
existen algunos que determinan el modo de ejecución y el estado del
procesador. A continuación se presenta el formato de estos registros.

Registro EFLAGS
---------------

Este registro almacena información del estado del procesador y configura
su ejecución. Tiene el formato que se presenta en la siguiente figura.

    Registro EFLAGS (32 bits)
        31               23              15              7             0
        +---------------------------------------------------------------+
        | | | | | | | | | | | |V|V| | | | | |   | | | | | | | | | | | | |
        |0|0|0|0|0|0|0|0|0|0|I|I|I|A|V|R|0|N|IO |O|D|I|T|S|Z|0|A|0|P|1|C|
        | | | | | | | | | | |D|P|F|C|M|F| |T|PL |F|F|F|F|F|F| |F| |F| |F|
        +---------------------------------------------------------------+
                             | | | | | |   |  |  | | | | | |   |   |   |
    ID Flag -----------------+ | | | | |   |  |  | | | | | |   |   |   |
    Virtual Interrupt Pending -+ | | | |   |  |  | | | | | |   |   |   |
    Virtual Interrupt Flag  -----+ | | |   |  |  | | | | | |   |   |   |
    Alignment Check ---------------+ | |   |  |  | | | | | |   |   |   |
    Virtual 8086 Mode ---------------+ |   |  |  | | | | | |   |   |   |
    Resume Flag -----------------------+   |  |  | | | | | |   |   |   |
    Nested Task ---------------------------+  |  | | | | | |   |   |   |
    I/O Privilege Level ----------------------+  | | | | | |   |   |   |
    Overflow Flag -------------------------------+ | | | | |   |   |   |
    Direction Flag --------------------------------+ | | | |   |   |   |
    Interrupt Flag ----------------------------------+ | | |   |   |   |
    Trap Flag -----------------------------------------+ | |   |   |   |
    Sign Flag -------------------------------------------+ |   |   |   |
    Zero Flag ---------------------------------------------+   |   |   |
    Auxiliary Carry Flag --------------------------------------+   |   |
    Parity Flag ---------------------------------------------------+   |
    Carry Flag --------------------------------------------------------+

Los bits del registro EFLAGS se pueden clasificar en:

- Bits de estado: Reflejan el estado actual del procesador. Son bits de
  estado: OF, SF, ZF, AF y PF.

- Bits de control: Controlan de alguna forma la ejecución del procesador.
  Dentro de EFLAGS se encuentra el bit DF, que permite controlar la
  dirección de avance en las operaciones sobre cadenas de caracteres.

- Bits del sistema: Los bits ID, VIP, VIF, AC, VM, RF, NT, IOPL, IF y TF
  son usados por el procesador para determinar condiciones en su
  ejecución, o para habilitar / deshabilitar determinadas características.
  Por ejemplo, estableciendo el bit IF en 1 se habilitan las
  interrupciones, mientras un valor de 0 en este bit deshabilita las
  interrupciones.

- Bits reservados: Estos bits se reservan por la arquitectura IA-32 para
  futura expansión. Deben permanecer con los valores que se muestran en la
  figura (cero o uno). No se deben usar, ya que es posible que en
  versiones posteriores de los procesadores IA-32 tengan un significado
  específico.

Registro CR0
------------

A continuación se ilustra el Control Register 0 (CR0). Este registro
controla aspectos vitales de la ejecución, como el modo protegido y la
paginación.

      Registro CR0 (32 bits)
        31               23        18 16 15              7             0
        +---------------------------------------------------------------+
        | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
        |P|C|N| | | | | | | | | | |A| |W| | | | | | | | | | |N|E|T|E|M|P|
        |G|D|W| | | | | | | | | | |M| |P| | | | | | | | | | |E|T|S|M|P|E|
        +---------------------------------------------------------------+
         | | |                     |  |                      | | | | | |
         +-|-|-- Paging            |  |                      | | | | | |
           | |                     |  |                      | | | | | |
           +-|-- Cache Disable     |  |                      | | | | | |
             |                     |  |                      | | | | | |
             +-- Non-write through |  |                      | | | | | |
                                   |  |                      | | | | | |
    Alignment Mask ----------------+  |                      | | | | | |
    Write Protect --------------------+                      | | | | | |
    Numeric Error -------------------------------------------+ | | | | |
    Extension Type --------------------------------------------+ | | | |
    Task Switched -----------------------------------------------+ | | |
    Emulation -----------------------------------------------------+ | |
    Monitor Coprocessor ---------------------------------------------+ |
    Protection Enable -------------------------------------------------+

Los bits más importantes de CR0 desde el punto de vista de programación
son el bit 0 (Protection Enable – PE), y el bit 31 (Paging – PG). Estos
permiten habilitar el modo protegido y la paginación, respectivamente.

No obstante antes de pasar a modo protegido y de habilitar la paginación
se deben configurar unas estructuras de datos que controlan la ejecución
del procesador.

Para habilitar el modo protegido se deberá tener configurada de antemano
una . Esta tabla deberá ser configurada por el cargador de arranque o
por el código inicial del kernel.

Organización de la Memoria
==========================

La memoria en los procesadores de arquitectura IA-32 se puede organizar
y manejar en tres formas básicas: Modo Segmentado, Modo Real de
Direcciones y Modo Plano Estas tres organizaciones permiten referenciar el 
espacio **lineal** de direcciones.. A continuación se muestran los detalles de
cada uno de estos modos.

Modo Segmentado
----------------

Este es el modo por defecto de organización de memoria. En este modo, la
memoria se aprecia como un conjunto de espacios lineales denominados
segmentos. Cada segmento puede ser de diferente tipo, siendo los más
comunes segmentos de código y datos.

Para referenciar un byte dentro de un segmento se debe usar una
dirección lógica , que se compone de un par selector: desplazamiento
(offset). El valor del selector se usa como índice en una tabla de
descriptores. El descriptor referenciado contiene la base del segmento,
es decir la dirección lineal del inicio del segmento.

El desplazamiento (offset) determina el número de bytes que se debe
desplazar desde el la base segmento. Así se obtiene una dirección lineal
en el espacio de direcciones de memoria.

Si el procesador tiene deshabilitada la paginación (comportamiento por
defecto), la dirección lineal es la misma dirección física (En RAM). En
el momento de habilitar la paginación, el procesador debe realizar un
proceso adicional para traducir la dirección lineal obtenida a una
dirección física. El offset se almacena en un registro de propósito
general, cuyo tamaño es de 32 bits de esta forma, el tamaño máximo de un
segmento es de 4GB.

A continuación se presenta una figura que ilustra cómo realiza este
proceso.

    Dirección Lógica                           Espacio lineal de direcciones
     +-------+  +---------------+         4 GB +--------------------+
     | sel   |  |  offset       |              |                    |
     +-------+  +---------------+              |                    | 
      selector    desplazamiento               |                    |
        |              |                       |                    | 
        |              |                       |                    |
        |              |                       |--------------------|--+         
        |              |                       |                    |  |    
        |              |                       |   Segmento de      |  | Tamaño
        |              |                       |   Memoria          |  | del
        |              |                       |                    |  | segmento
        |              +---------------------> | Dirección Lineal   |  | (límite)
        |                                      |        ^           |  |
        |              +                       |        |           |  |
        |                                      |        |           |  |
        +------------ base   ----------------->|--------|-----------|--+ Base del
            Con el selector se halla           |        |           |    segmento
            la "base" del segmento             |        |           |      ^
            (Su dirección de inicio en la      |        |           |      |
             memoria)                          |        |           |      |
                                               |        |           |      |
                                             0 +--------------------+      -


Modo Real de Direcciones
------------------------

El modo real de direcciones es un caso especial del modo segmentado, que
se usa cuando el procesador se encuentra operando en Modo Real. Se usa
para ofrecer compatibilidad con programas desarrollados para
generaciones anteriores de procesadores, que abarcan hasta 8086. 

En modo real de direcciones el espacio lineal de direcciones se
encuentra dividido en segmentos con un tamaño máximo de 64 Kilobytes.
Esto se debe a que cuando el computador opera en modo real, sólo es sólo
es posible usar los 16 bits menos significativos de los registros de
propósito general, que se usan para almacenar el desplazamiento de la
dirección lineal dentro del segmento.

Las direcciones lógicas en modo real también están conformadas por un
selector y un offset. Tanto el selector como el desplazamiento tienen un
tamaño de 16 bits. Con el fin de permitir el acceso a un espacio de
direcciones lineal mayor, el selector almacena la dirección de inicio
del segmento dividida en 16. 

Para traducir una dirección lógica a
lineal, el procesador toma el valor del selector y lo multiplica
automáticamente por 16, para hallar la base del segmento. Luego a esta
base le suma el offset, para obtener una dirección lineal de 20 bits.
Así, en modo real sólo se puede acceder al primer MegaByte de memoria.
La siguiente figura ilustra el proceso de transformar una dirección
lógica a lineal en el modo real de direcciones.

     Dirección Lógica                           Espacio lineal de direcciones
     +-------+  +---------------+         1 MB +--------------------+
     |base/16|  |   offset      |              |                    |
     +-------+  +---------------+              |                    | 
      selector    desplazamiento               |                    |
        |              |                       |                    | 
        |              |                       |                    |
        |              |                       |--------------------|--+         
        |              |                       |                    |  |    
        |              |                       |   Segmento de      |  | Tamaño
        |              |                       |   Memoria          |  | del
        |              |                       |                    |  | segmento
        |              +---------------------> | Dirección Lineal   |  | (límite)
        |                                      |        ^           |  | Máx 64 KB
        |              +                       |        |           |  |
        |                                      |        |           |  |
        +------------ base   ----------------->|--------|-----------|--+ Base del
            El selector almacena la base       |        |           |    segmento
            del segmento dividida en 16.       |        |           |      ^
            El procesador multiplica el        |        |           |      |
            valor del selector por 16 para     |        |           |      |
            hallar la base del segmento.       |        |           |      |
                                             0 +--------------------+      -

Modo Plano (Flat)
-----------------

El modo plano es otro caso especial del modo segmentado. La memoria en
este modo se presenta como un espacio continuo de direcciones (espacio
lineal de direcciones). Para procesadores de 32 bits, este espacio
abarca desde el byte 0 hasta el byte 2**32 - 1 (4GB). En la práctica,
el modo plano se puede activar al definir segmentos que ocupan todo el
espacio lineal (con base = 0 y un tamaño igual al máximo tamaño
disponible).

Dado que en este modo se puede ignorar la base del segmento (al
considerar que siempre inicia en 0), el desplazamiento en una dirección
lógica es igual a la dirección lineal (Ver figura).

     Dirección Lógica                           Espacio lineal de direcciones
     +-------+  +---------------+         4 GB +--------------------+-+
     | sel   |  |   offset      |              |                    | |
     +-------+  +---------------+              |                    | |
      selector    desplazamiento               |                    | |
        |              |                       |                    | |
        |              |                       |                    | |
        |              |                       |                    | |          
        |              |                       |                    | |     
        |              |                       |   Segmento de      | |  Tamaño
        |              |                       |   Memoria          | |  del
        |              |   offset              |                    | |  segmento
        |              +---------------------> | Dirección Lineal   | |    =
        |                                      |  = offset          | |  Tamaño 
        |              +                       |        ^           | |  del 
        |             base = 0                 |        |           | |  espacio
        |                                      |        |           | |  lineal
        |   En el modo plano (flat), el        |        |           | |  de      
        |   segmento tiene como base 0 y como  |        |           | |direcciones
        |   límite el tamaño del espacio       |        |           | |     
        |   lineal de direcciones.             |        |           | |     
        |                                      |        |           | |     
        +----------------------------------->0 +--------------------+-+

Ver también
======
- [Manuales de Intel](http://www.intel.com/content/www/us/en/processors/architectures-software-developer-manuals.html)
