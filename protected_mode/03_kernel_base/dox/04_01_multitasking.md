Multitarea
==========

El Sistema Operativo deberá inicializar un entorno en el cual las tareas se
puedan ejecutar. Este entorno está constituido por:

1. Un Segmento de Estado de Tarea (TSS) referenciado por un Descriptor dentro de
   la GDT y por su respectivo selector.
2. Dos descriptores Descriptores de segmento de código y datos dentro de la GDT
   par la tarea y su respectivos  selectores, que describen la organización en
   memoria de la tarea.
3. Un mecanismo para crear una tarea a partir de un archivo ejecutable
4. Un mecanismo para pasar el control de la CPU a las tareas
5. Un mecanismo para retornar el control de la CPU al sistema operativo
6. Un mecanismo de comunicación entre las tareas y el sistema operativo.

A este entorno se suman los descriptores de código y datos del kernel,
necesarios para operar en modo protegido de 32 bits.

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
  - Ejecución : La tarea tiene asignada la CPU y se están ejecutando sus
    instrucciones
  - Bloqueado: La tarea se encuentra esperando a que suceda un evento para
    continuar su ejecución
  - Terminado : La tarea ha terminado la ejecución y está lista para que el
    sistema operativo libere los recursos asociados a ella
- Nivel de privilegios de la tarea, que puede estar asociado a los niveles
  de privilegios del procesador o a una jerarquía de privilegios definida
  por el sistema operativo
- Dirección relativa a la sección de código de la tarea en la cual se debe
  comenzar su ejecución
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


Mecanismo para la Creación de una Tarea desde un Archivo Ejecutable
-------------------------------------------------------------------


Para crear una tarea a partir de un archivo ejecutable, se debe tener en
cuenta varios aspectos:

- Formato Ejecutable: Existe una gran cantidad de formatos ejecutables. En estos
	proyectos se usa el formato ELF, que es el formato por defecto de los
	archvos ejecutables de los sistemas operativos Linux.
- Lectura de los Archivos Ejecutables: En primera instancia los archivos
	ejecutables se encontrarán almacenados en la misma imagen de disco en la
	cual se carga el kernel. Por lo tanto se tienen dos alternativas para leer
	los archivos:
		- Implementar dentro del kernel los drivers del dispositivo, y las
			rutinas para el manejo de sistemas de archivos ext2 (formato de la
			partición en la cual se encuentra el kernel).
		- Usar las facilidades que ofrece GRUB para cargar otros archivos además
			del kernel: módulos adicionales o ramdisk. En los proyectos se usa
			la alternativa de cargar módulos adicionales junto con el kernel.

Después de tomar en cuenta las consideraciones anteriores, se debe implementar
la lógica para crear la tarea a partir de los archivos ejecutables ELF cargados
como módulos adicionales junto con el kernel.

Carga de módulos de kernel
--------------------------

La carga de módulos adicionales del kernel es relativamente simple: se debe
indicar a GRUB en su archivo de configuración la ubicación y los
parámetros que se desean pasar a cada módulo. Por ejemplo: 

    default 0
    timeout 10
    color cyan/blue white/blue
    
    title Aprendiendo Sistemas Operativos
    root (hd0,0)
    kernel /boot/kernel
    
    module /boot/module0.elf
    module /boot/module1.elf 1 2 3
    module /boot/module2.elf 1 2 3 4 5


En este caso se cargan tres módulos adicionales de kernel (/boot/module0.elf,
/boot/module1.elf y /boot/module2.elf), y a cada uno de ellos se pasa una serie
de parámetros diferentes. Estos módulos deberán estar copiados dentro de la
imagen de disco, preferiblemente en el mismo directorio del kernel.

GRUB cargará el kernel (/boot/kernel) en la posición de memoria definida en el
encabezado multiboot. Luego cargará los módulos en memoria a continuación del
kernel. Además, GRUB actualizará la estructura de datos Multiboot que le pasa
como parámetro al kernel. Recordando la Estructura de Información Multiboot:

             +-------------------+
           0 | flags             | (required)| Permite identificar cuales de los  
             |                   | siguientes campos se encuentran definidos:
             +-------------------+
           4 | mem_lower         | 
           8 | mem_upper         |
             +-------------------+
          12 | boot_device       | 
             +-------------------+
          16 | cmdline           |
             +-------------------+
          20 | mods_count        | <-- Número de módulos  y dirección de memoria en
          24 | mods_addr         |  la cual se cargó la información de cada módulo
             +-------------------+
     28 - 40 | syms              | 
             |                   | 
             +-------------------+
          44 | mmap_length       | 
          48 | mmap_addr         | 
             +-------------------+
          52 | drives_length     | 
          56 | drives_addr       | 
             +-------------------+
          60 | config_table      | 
             +-------------------+
          64 | boot_loader_name  | 
             +-------------------+
          68 | apm_table         | 
             +-------------------+
          72 | vbe_control_info  | 
          76 | vbe_mode_info     |
          80 | vbe_mode          |
          82 | vbe_interface_seg |
          84 | vbe_interface_off |
          86 | vbe_interface_len |
             +-------------------+
    
En la dirección de memoria especificada por mods_addr, se encuentra un arreglo
de mods_count entradas, cada una de las cuales describe un módulo cargado en
memoria. 

        +-------------------+
     0  | mod_start         |
     4  | mod_end           |
        +-------------------+
     8  | string            |
        +-------------------+
     12 | reservado (0)     |
        +-------------------+
    
Los parámetros mod_start y _mod_end especifican la posición de memoria del
módulo, y el parámetro string especifica la línea de comandos especificada en el
archivo de configuración de GRUB.

Archivos Ejecutables en formato ELF
------------------------------------

Un archivo ejecutable en formato ELF contiene en su inicio un encabezado que
describe su estructura y los requerimientos para su ejecución.

         Archivo ELF
       +------------------------+
       |Encabezado ELF          |
       +------------------------+<------+
       |Encabezado de Programa  |<--+   | Tabla de Encabezados de Programa
       +------------------------+   |   | Describe las secciones del programa
       |Encabezado de Programa  |<----+ | que se usan usan en su ejecución
       +------------------------+<--|-|-+    
       | Sección 1              |<--+ |
       | .text                  |   | |
       +------------------------+   | |
       | Sección 2              |<--+ |
       | .rodata                |     |
       +------------------------+     |
       | Sección 3              |<----+
       |  .data                 |
       +------------------------+ <----+
       |Encabezado de Sección   |      |
       +------------------------+      | Tabla de Encabezados de Sección
       |Encabezado de Sección   |      | Describe todas las secciones del
       +------------------------+      | programa, aún si no se usan en
       |Encabezado de Sección   |      | su ejecución.
       +------------------------+ <----+
    
A partir de la información del formato ELF es posible extraer las secciones de
código y datos del programa, y organizarlas en otra posición de memoria junto
con la pila y el espacio para asignación dinámica de memoria de la tarea.

Cada sistema operativo organiza sus tareas de forma diferente. No obstante se
pueden indentificar componentes básicos que una tarea debe tener. Estos se
ilustran en el siguientes esquema.

       Ubicación de la Tarea en Memoria
       +--------------------------------+
       |                                |
       | Memoria disponible para otras  |
       | tareas o para el Sistema       |
       | Operativo                      |
       |                                |
       |                                |
       |                                |
       |                                |
       |                                |  
       +--------------------------------+ <-- Tope de la pila para la tarea y
       |                                |     fin del área de memoria asignada a
       |                                |     la tarea
       |   Pila usable por la tarea     |
       |                                | (la pila crece hacia abajo)
       |                                |
       |--------------------------------| <-- Base de la pila para la tarea
       |  Espacio para asignación       | 
       |  dinámica de memoria de la     |  Espacio para el heap de la tarea
       |         tarea (heap)           |
       |--------------------------------| <-- Base del heap para la tarea
       |                                |
       |     Datos de la tarea          |
       |--------------------------------| <-- Fin de la sección de código
       |                                |
       |     Código de la tarea         |
       |                                |
       +--------------------------------+ < -- dir. lineal de inicio de la tarea
       | Memoria disponible             |
       |                                |
       |                                |
       +--------------------------------+ <-- Inicio de la región de memoria   
       | Pila del Sistema Operativo     |     disponible para asignar a las
       |                                |     tareas
       +--------------------------------+
       |                                |
       | Datos del Sistema Operativo    |
       |                                |
       |                                |
       +--------------------------------+
       | Código del Sistema Operativo   |
       |                                |
       |                                |
       +--------------------------------+
    
Si no se usa memoria virtual, la región de memoria de la tarea debe ser un
espacio continuo del cual se requiere conocer:

- La Base ( dirección de inicio) y Límite (tamaño de la región asignada a
  la tarea)
- Los desplazamientos a partir del inicio de la tarea en los cuales:
	- Termina su sección de código
	- Comienza y termina su sección de datos
	- Comienza y termina su región de asignación dinámica de memoria (heap)
	- Comienza y termina su pila.

Si se usa memoria virtual con paginación, la región de memoria de la tarea no
necesariamente debe estar ubicada de forma contigua en la memoria física, pero
sí debe ser contigua en su espacio lineal de memoria.

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

Llamadas al sistema
--------------------

El mecanismo básico de comunicación entre las tareas y el Sistema Operativo es
la interrupción. Las tareas almacenan en los registros del procesador o su pila
los parámetros que se desean pasar al sistema operativo, y luego invocan una
interrupción específica. El sistema operativo recibe entonces el control por
medio de la rutina de manejo de interrupción, obtiene el apuntador a la pila en
la posición en la cual la rutina ISR almacenó el estado del procesador, y con
esta información ejecuta la funcionalidad requerida.

Este mecanismo es similar a la comunicación que se realiza en modo real con la
BIOS: se establecen los registros del procesador en determinados valores, y
luego se invoca una interrupción.

Los sistemas operativos modernos también usan esta estrategia. Por ejemplo, la
comunicación entre las tareas y el kernel de Linux se realiza a través de la
interrrupción 0x80.

