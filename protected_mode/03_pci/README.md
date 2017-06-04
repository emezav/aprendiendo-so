Peripheral Component Interconnect
================================

Este proyecto implementa el código para detectar los dispositivos que se
encuentran conectados a los buses del sistema mediante la interfaz PCI.

Para este propósito, primero se verifica si el sistema cuenta con uno o más
buses PCI. Luego, para cada bus, se detectan los dispositivos y las
funciones disponibles. Con la información recopilada se crea un arreglo con
máximo 256 entradas para los diferentes dispositivos detectados.

En caso de no encontrar buses PCI, el kernel detiene su ejecución.

Breve introducción a PCI
===========================
La interfaz PCI fue propuesta por Intel en los años 90 para conectar
diferentes dispositivos periféricos a la tarjeta madre del computador,
permitiendo una mayor velocidad de intercambio de datos entre los
dispositivos, el procesador y la memoria. Esta interfaz reemplazó la
interfaz ISA como el estándar de conexión de elementos de
hardware en los computadores personales.

Un sistema computacional puede tener hasta 256 buses PCI, cada uno con 32
dispositivos conectados.  Todo dispositivo PCI implementa como mínimo la
función 0. Como máximo cada dispositivo puede tener 8 funciones. Por
ejemplo, un dispositivo que combina Wi-fi y Bluetooth tendrá dos funciones,
función 0 y función 1.

Para interconectar buses PCI se usan Puentes PCI-PCI (PCI Bridges). También
es posible conectar dispositivos PCMCIA mediante Puentes Cardbus.

Aproximadamente en el año 2002 se propuso la interfaz PCI Express, la cual
extendió las capacidades de PCI para usarse en diferentes tipos de sistemas
(computadores personales, servidores, dispositivos móviles, etc.), pero
permite operar usando los elementos básicos de la especificación original.
De hecho, una de sus capacidades más importantes desde el punto de vista de
compatibilidad radica en la posibilidad de usar los mecanismos de
detección y configuración de hardware PCI Express usando las
implementaciones de software basadas en PCI, sin modificaciones.

Espacio de configuración PCI
==============================
Todos los dispositivos conectados a un bus PCI deben proporcionar un
__Espacio de Configuración__.  Este espacio se divide en 64 __registros__
de 32 bits (4 bytes) cada uno, para un total de 256 bytes.
PCI Express extendió el espacio a 4096 bytes, manteniendo el formato
original de los primeros 256 para ofrecer compatibilidad con los
dispositivos *heredados* (legacy).

Los registros iniciales del espacio de configuración conforman el
*encabezado*, el cual es común a todos los dispositivos. Dentro de este
encabezado existe un __registro__ llamado *Header Type*, el cual permite
clasificar el dispositivo en tres clases diferentes. Los bits 0-6 de este
campo definen el tipo de dispositivo:

- Tipo 0x00: Dispositivo PCI.
- Tipo 0x01: Puente PCI-PCI.
- Tipo 0x02: Puente Cardbus.

Todos los dispositivos deben implementar los campos Vendor ID, Device ID,
Command, Status, Revision ID, Class Code, y Header Type. Además, todos los
dispositivos con *Header Type* 0x00 deben implementar los primeros bytes
del espacio de configuración con el siguiente formato:

          31               15         7            0
   offset +----------------------------------------+
      0x00|   Device ID    |   Vendor ID           |
          |                |                       | 
      0x04|   Status       |   Command             |
          |                |                       | 
      0x08|   Class Code              | Revision   |
          |                           |   ID       | 
      0x0C|BIST   |Header  |Latency   | Cacheline  |
          |       | Type   | Timer    |   Size     |
      0x10|Base Address Register #0 (Bar0)         |
          |                                        | 
      0x14|Base Address Register #1 (Bar1)         |
          |                                        | 
      0x18|Base Address Register #2 (Bar2)         |
          |                                        | 
      0x1C|Base Address Register #3 (Bar3)         |
          |                                        | 
      0x20|Base Address Register #4 (Bar4)         |
          |                                        | 
      0x24|Base Address Register #5 (Bar5)         |
          |                                        | 
      0x28|      Cardbus CIS Pointer               |
          |                                        | 
      0x2C|Subsystem ID    | Subsystem Version     |
          |                |        ID             | 
      0x30|Expansion ROM Base Address              |
          |                                        | 
      0x34|Reserved                   |Capabilities|
          |                            Pointer     | 
      0x38|Reserved                                |
          |                                        | 
      0x3C|Max_Lat|Min_Gnt |Interrupt | Interrupt  |
          |                | Pin      |  Line      | 
          +----------------------------------------+
     
El campo *Header type* también permite identificar la configuración de la
segunda parte del encabezado (a partir del byte 0x10).

El campo *Class Code* se subdivide en tres: El más significativo
corresponde a la clase del dispositivo, que especifica de forma general el
tipo de función que cumple el dispositivo. El segundo más significativo
define de forma más específica la función, y el menos significativo define
la interfaz de programación del dispositivo.

Las clases de dispositivo de acuerdo con la especificacion PCI 2.3 son:

    Clase     Descripción 
		-----     ---------------------------------------------------
    0x00      Devices built before class codes (i.e. pre PCI 2.0)
    0x01      Mass storage controller
    0x02      Network controller
    0x03      Display controller
    0x04      Multimedia device
    0x05      Memory Controller
    0x06      Bridge Device
    0x07      Simple communications controllers
    0x08      Base system peripherals
    0x09      Inupt devices
    0x0A      Docking Stations
    0x0B      Processorts
    0x0C      Serial bus controllers
    0x0D-0xFE Reserved
    0xFF      Misc

Los códigos de sub-clase e interfaz pueden ser consultados en el Apéndice D
de la Especificación PCI 2.3. Una lista breve también se encuentra en la
siguiente URL: https://pci-ids.ucw.cz/read/PD/

La combinación de los campos Vendor ID, Device ID, clase, subclase e
interfaz se pueden usar para implementar drivers específicos para cada
dispositivo conectado.

Detección de dispositivos conectados
------------------------------------
La mayoría de sistemas cuentan con dos buses PCI (bus 0 y bus 1), en
los cuales se conectan todos los dispositivos. Sin embargo, si se desea
realizar una detección exhaustiva, se debe verificar cada posición en la
cual se puede conectar un dispositivo, es decir cada *slot* (0-31), de
cada uno de los 256 buses posibles (0-255).

Por lo general, un dispositivo implementa una sola *función* (la función
0). Sin embargo, es posible contar con dispositivos multi-función (modem -
tarjeta de sonido, Wifi - Bluetooth), por lo tanto, se debe verificar las 8
funciones posibles (0-7).

Cuando un slot se encuentra vacío (no tiene un dispositivo conectado), se
obtendrá 0xFFFF en el campo Vendor ID, es decir, en los 16 bits menos
significativos que se obtienen al leer el registro 0 en el espacio de
configuración del dispositivo correspondiente.

El pseudocódigo para detectar los dispositivos conectados puede ser el
siguiente:
    
    Para cada b en 0-255 //Bus
           Para cada d en 0-31 //Slot (dispositivo)
                    Para cada f en 0-7 //Función
                        //Leer Registro 0: Device ID, Vendor ID
                        Leer Device ID, Vendor ID en Registro 0 de b, d, f
                        si Vendor ID = 0xFFFF
                            break //dispositivo no conectado
                        sino
														//Leer registros 0-16 de b, d, f
                            si el bit 7 del Header Type = 0 //única función
																//y almacenar información del dispositivo
                                break //No es necesario revisar las demás funciones
                            fin si
                        fin si
                    Fin para
              Fin para
        Fin para

Acceso al espacio de configuración mediante operaciones de E/S
----------------------------------
En los computadores personales, el acceso al espacio de configuración se
puede realizar mediante un par de registros de 32 bits, llamados Registro
de configuración (0xCF8), y Registro de Datos (0xCFC).  El acceso se
debe realizar en dos pasos: Primero se debe escribir en el registro de
configuración para seleccionar el dispositivo PCI sobre el cual se
realizará la transacción, y luego se deberá leer o escribir en el registro
de datos el valor que se va a leer o escribir en el espacio de
configuración.

Para acceder a una función de un dispositivo, se debe escribir un valor
de 32 bits en el registro de configuración (0xCF8), en el cual se codifican
el bus, el slot y la función a consultar. El formato del valor a escribir
en el registro de configuración es el siguiente:

    31 30          2423            1615      1110   8 7         2 1 0
         +---------------------------------------------------------------+
     |1| Reservado   |      # del    | # del   |# de | # del     |0|0|
     | |             |       Bus     |Disposit.|Func.| Registro  | | |
         +---------------------------------------------------------------+

Dado que se usan 5 bits para referenciar  el número del registro, con este
mecanismo se pueden acceder los primeros 64 registros de 4 bytes cada uno.

Luego de escribir el valor en el registro de configuración que permite
seleccionar el bus, el dispositivo, la función y el número de registro, se
debe leer el valor obtenido del registro de datos (0xCFC). Por ejemplo,
para leer el registro que contiene el Device ID y el Vendor ID (registro 0)
del dispositivo conectado en el bus 0, slot 0, función 0, se deberá
escribir el siguiente valor en el registro de configuración:

    31 30          2423            1615      1110   8 7         2 1 0
         +---------------------------------------------------------------+
     |1|  00000000   |   00000000    | 00000   | 000 |  00000    |0|0|
         +---------------------------------------------------------------+

Acceso al espacio de configuración en memoria
--------------------------------------
PCI Express definió otro mecanismo para acceder al espacio de configuración.
El Firmware (BIOS o UEFI) o el sistema operativo pueden mapear todo el
espacio de configuración a una dirección de memoria determinada. Dado que
cada función de un dispositivo ocupa 4096 bytes (4 KB), el firmware debe
mapear un espacio de 256 MB (256 buses * 32 slots * 8 funciones) * 4 KB. Al
inicio de este espacio se encontrará el espacio de configuración de la
función 0 del slot 0 del bus 0, luego la función 1 del slot 0 del bus 0, y
así sucesivamente.

Detección de la interfaz PCI en sistemas con PCI
------------------------------
En sistemas con PCI, se puede usar los servicios de la BIOS mientras se
está operando en modo real para obtener la información PCI. La función a
usar es int 0x1A, ax= 0x101 (Installation check). La descripción de este
servicio extendido se puede encontrar en Ralph Brown's Interrupt List:
http://www.ctyme.com/intr/rb-2371.htm. Esta función permite, entre otros
parámetros, determinar el número del último bus PCI presente en el sistema.

Otra estrategia consiste en leer el valor almacenado en el puerto de I/O
correspondiente al Registro de Configuración PCI (0xCF8), el cual contendrá
la última transacción realizada posiblemente por el Firmware. En caso de no
existir la interfaz PCI, se obtendrá como resultado el valor 0xFFFFFFFF.

Detección de la interfaz PCI en sistemas con PCI Express
------------------------------
En sistemas con PCI Express, se debe usar la configuración proporcionada
por el subsistema ACPI. El proceso básico es el siguiente:

- Ubicar el RSDP, el apuntador a la Tabla Raíz de Descriptores del Sistema
    (RSDT). Este apuntador almacena la dirección física de la RSDT.
- Buscar dentro de la RSDT la entrada correspondiente a la Configuración de
        Mapeo de Memoria de PCI (MCFG). Esta entrada corresponde a la dirección
        en donde se encuentra mapeado el espacio de configuración de PCI.

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
- Ralph Brown's Interrupt List http://www.ctyme.com/intr/rb-2371.htm
- ACPI - Advanced Configuration and Power Interface http://www.uefi.org/acpi/specs
- PCI Special Interest Group - PCI SIG http://www.pcisig.com
- Jeff Leyda. PCI bus info and code from a programmer's perspective. https://www.waste.org/~winkles/hardware/pci.htm
- PCI Vendor and Device Lists http://pcidatabase.com
- David Jurgens, Help-PC Reference Library http://stanislavs.org/helppc/idx_interrupt.html
