Descripción general del proyecto
================================



Este kernel realiza varias tareas:

- Haciendo uso de la Estructura de Información Multiboot obtenida de GRUB,
    configura y habilita la memoria virtual. Para ello, se mapea el primer
    megabyte de memoria y el kernel (que fue cargado en la dirección física
    0x100000 - 1 MB) a la dirección 0xC0000000 (3 GB). En esta ubicación
    virtual residirá el kernel. Este proceso se inicia en el archivo
    kernel/src/start.S y se completa en el archivo kernel/src/kernel.c
    haciendo uso de las funciones implementadas en kernel/src/paging.c.
- Después de configurar y habilitar la memoria virtual, se carga una nueva
    Tabla Global de Descriptores (GDT), en el espacio de memoria del
    kernel. Esta GDT define los descriptores de segmento de código y datos
    para el kernel.
- Posteriormente, se configura un mapa de bits que permite la gestión de la
    memoria física en unidades de 4 KB (el mismo tamaño de las páginas).
    Este proceso se realiza en el archivo kernel/src/kernel.c,  con ayuda
    de las funciones implementadas en kernel/src/physmem.h. En estas
    funciones, se hace uso de la información de las regiones de memoria que
    recopiló GRUB.
- Finalmente, se configura la Tabla de Descriptores de Interrupción (IDT), para
    recibir las excepciones del procesador (interrupciones 0 - 31), y las
    solicitudes de interrupción (IRQ) de los dispositivos de entrada y
    salida (interrupciones 32 - 48). Para ello es necesario reprogramar el
    Programmable Interrupt Controller (PIC). Este proceso lo realiza la
    función setup_interrupts(), invocada en el archivo kernel/src/kernel.c.
    La rutina setup_interrupts(), las subrutinas y estructuras de datos
    relacionadas se encuentran en los archivos kernel/src/interrupt.c,
    kernel/src/irq.c y sus respectivos archivos de cabecera.

Cuando completa su inicialización, el kernel se encuentra ejecutándose en
memoria virtual a partir de la dirección 0xC100000, se ha configurado la
GDT y la IDT. 

A partir de este momento, cualquier excepción del procesador (incluida el
fallo de página) o cualquier solicitud de interrupción (IRQ) de los
dispositivos, hace que el kernel imprima un mensaje y entre en una espera
activa. Se debe continuar implementando:
- Las rutinas de manejo de IRQ para los dispositivos de entrada y salida,
    específicamente el timer y el teclado.
- La gestión de los fallos de página, ocasionados por referencias a páginas
    que no se encuentran mapeadas a ningún marco.

Estructura de la Información Multiboot recopilada por GRUB
----------------------------------------------------------

La estructura de datos que GRUB construye y cuyo apuntador se almacena en 
el registro EAX es definida por la Especificación Multiboot con el formato 
que se presenta a continuación. El desplazamiento se encuentra 
definido en bytes, es decir que cada campo ocupa 4 bytes (32 bits).

             +-------------------+
           0 | flags             | (required)| Permite identificar cuales de los
             |                   | siguientes campos se encuentran definidos:
             +-------------------+
           4 | mem_lower         | (presente si flags[0] = 1)
           8 | mem_upper         | (presente si flags[0] = 1)
             +-------------------+
          12 | boot_device       | (presente si flags[1] = 1)
             +-------------------+
          16 | cmdline           | (presente si flags[2] = 1)
             +-------------------+
          20 | mods_count        | (presente si flags[3] = 1)
          24 | mods_addr         | (presente si flags[3] = 1)
             +-------------------+
     28 - 40 | syms              | (presente si flags[4] or
             |                   | flags[5] = 1)
             +-------------------+
          44 | mmap_length       | (presente si flags[6] = 1)
          48 | mmap_addr         | (presente si flags[6] = 1)
             +-------------------+
          52 | drives_length     | (presente si flags[7] = 1)
          56 | drives_addr       | (presente si flags[7] = 1)
             +-------------------+
          60 | config_table      | (presente si flags[8] = 1)
             +-------------------+
          64 | boot_loader_name  | (presente si flags[9] = 1)
             +-------------------+
          68 | apm_table         | (presente si flags[10] = 1)
             +-------------------+
          72 | vbe_control_info  | (presente si flags[11] = 1)
          76 | vbe_mode_info     |
          80 | vbe_mode          |
          82 | vbe_interface_seg |
          84 | vbe_interface_off |
          86 | vbe_interface_len |
             +-------------------+

Consulte la Especificación Multiboot para obtener más detalles acerca de esta
estructura.

Ejecución del Kernel
--------------------

La ejecución del kernel se divide en dos partes: ejecución del código
inicial (programado en lenguaje ensamblador) y ejecución del código en C.

Ejecución del código inicial del Kernel
---------------------------------------

El código del kernel se encuentra organizado de forma que primero se ejecuta
el código del archivo start.S. Esta organización del archivo ejecutable se
define en el archivo link.ld.

El código de start.S define el encabezado multiboot, necesario para que
el kernel sea reconocido por GRUB como compatible con la especificación
multiboot. Este encabezado es leído e interpretado por GRUB al momento de
cargar el kernel, de acuerdo con lo establecido en la especificación.

Inicialmente, el código en start.S valida si el kernel fue cargado por un
cargador de arranque compatible con la Especificación Multiboot, en cuyo caso el
registro EAX contendrá el valor 0x2BADB002, que indica que el cargador de
arranque es compatible con Multiboot. Además el registro EBX almacenará la
posición en memoria de la Estructura de Información Multiboot, que puede ser
usada por el kernel para configurar el mapa de bits de la memoria disponible.

Antes de pasar el control a la rutina cmain definida en el archivo kernel.c, el
código en ensamblador de start.S se encarga de crear y configurar las
estructuras de datos necesarias para habilitar la paginación. Se crea un
directorio de tablas de página y tantas tablas de página como sean necesarias
para mapear el primer megabyte de memoria física y el kernel en memoria virtual.

Luego de crear el directorio de tablas de página y las tablas de página
necesarias para el kernel y sus estructuras de datos, se mapea el kernel
tanto en la parte baja de la memoria como en la parte alta, como se muestra
en la siguiente figura:


    Espacio lineal (virtual)
    +-----------------------+
    |                       |
    |Tablas de página       | 
    |Dir. tablas de página  | 
    |Mapa de bits de memoria|         
    |Kernel (Codigo y datos)|
    |-----------------------|<= 0xC0100000: inicio del kernel
    | (1 MB)                |
    |-----------------------|<= 0xC0000000 (3 GB)
    |                       |
    |                       |
    |                       |
    |                       |
    |                       |
    |                       |             Espacio físico (RAM)
    |                       |     +------------------------+
    |                       |     |                        |
    |                       |     | RAM DISPONIBLE         |
    |                       |     |                        |
    |-----------------------|     |------------------------|
    |Tablas de página       |     |Tablas de página        |
    |Dir. tablas de página  |     |Dir. tablas de página   |
    |Mapa de bits de memoria|     |Mapa de bits de memoria |
    |Kernel(codigo y datos) |     |Kernel (codigo y datos) |
    |-----------------------|     |------------------------| <= 0x100000
    | (1 MB)                |     |  (1 MB)                |
    +-----------------------+ 0   +------------------------+

Este doble mapeo es necesario, dado que según el Manual de Intel 
(IA-32 Volumen 3A, sección 17.29.3), es necesario que la secuencia de
instrucciones necesarias para modificar el registro CR3 se debe realizar
en una página mapeada a identidad, es decir, que su dirección lineal y
su dirección física sean idénticas.

Posteriormente se debe cargar de nuevo una GDT, configurada en la
nueva ubicación del kernel.

Finalmente, el código de start.S transfiere el control
a la función __cmain__ definida en el archivo kernel/src/kernel.c.

Ejecución del código en C del Kernel 
------------------------------------
La subrutina __cmain__ completa el proceso de inicialización, quitando
el mapeo a identidad del kernel. De este punto en adelante, el kernel
se ejecutará en "la parte alta" de la memoria (virtual).

El siguiente paso de la inicialización consiste en inicializar el mapa
de bits que manejará la memoria disponible, en regiones de 4 KB (el 
tamaño de una página.) Esta tarea la realiza la función setup_physical_memory,
implementada en el archivo kernel/src/physmem.c. La configuración del mapa
de bits se realiza recuperando la información proporcionada por GRUB, y buscando
la región de memoria física por encima del kernel, que se encuentre marcada
como disponible.

Finalmente, la subrutina setup_interrupts definida en kernel/src/interrupt.c
se encarga de realizar la lógica necesaria para configurar la IDT y definir
un manejador genérico para todas las interrupciones. También se define un
manejador genérico para cada excepción del procesador (ver
kernel/src/exception.c), y se realiza la configuración de las solicitudes
de interrupción de hardware (ver kernel/src/irq.c).  

El kernel finaliza imprimiendo un mensaje por consola.

Cuando la función cmain() termina, el control retorna de nuevo al código del 
archivo start.S. Este código retira los parámetros almacenados en la pila, y
entra en un ciclo infinito para mantener ocupado el procesador y evitar que 
éste se reinicie:
      /* La función cmain() retorna a este punto. Se debe entrar en un ciclo
      infinito, para que el procesador no siga ejecutando instrucciones al 
      finalizar la ejecución del kernel. */
    
    loop:	hlt
    	jmp loop /* Ciclo infinito */
      


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
