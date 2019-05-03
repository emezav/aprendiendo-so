Descripción general del proyecto
================================

El kernel creado en este proyecto es compatible con la Especificación
Multiboot. Todos los archivos que lo componen se compilan en un único archivo
en formato ELF (el formato ejecutable de Linux) y se almacena en una imagen de 
disco duro. Esta imagen de disco ya tiene integrado a GRUB.

GRUB es un cargador de arranque que cumple con la Especificación Multiboot. De
esta forma, si se desarrolla un kernel compatible con Multiboot, podrá ser 
cargado por GRUB sin ningún inconveniente. 

El archivo ejecutable del kernel se organiza para que sea cargado por GRUB a la 
dirección de memoria 0x100000, y para que su ejecución comience en la etiqueta 
*start* del archivo src/start.S.

Imagen de disco del proyecto
----------------------------

La  imagen de disco ya contiene a GRUB en su sector de arranque maestro, y 
tiene una sola partición. Dentro de esta partición se encuentran los archivos 
necesarios para iniciar GRUB, y el código del kernel compilado. 

La partición se encuentra formateada con el sistema de archivos ext2 (linux), 
y su contenido se muestra en el siguiente esquema:

    / -+    <-- Directorio raíz de la única partición en la imagen de disco
       |
       boot <-- Almacena los archivos de GRUB y el kernel compilado
           |  
           |  
           grub  <-- Almacena los archivos de GRUB
           |   |
           |   |
           | e2fs_stage_1_5  <-- Etapa 1.5 de GRUB. cargado por la etapa 1 de 
           |   |                 GRUB.
           |   |                 Contiene el código para manejar el sistema de 
           |   |                 archivos de la partición (ext2). Este archivo
           |   |                 es opcional, ya que al instalar GRUB este
           |   |                 archivo se copió en el sector adyacente al
           |   |                 sector de arranque.
           |   |
           | menu.lst  <-- Archivo de configuración leido por GRUB al arranque.
           |   |             especifica la configuración del menú que despliega
           |   |             GRUB al arranque y ubicación del kernel en 
           |   |             (la imagen de) disco.
           | stage1    <-- Etapa 1 de GRUB. Este archivo es opcional, ya que se
           |   |           copió en el sector de arranque del disco al instalar 
           |   |           GRUB.
           |   |           Carga la etapa 1.5 de GRUB. Después carga la
           |   |           etapa 2 de GRUB desde el disco y le pasa el control.
           |   |           Este archivo es opcional.
           |   |
           | stage2    <-- Etapa 2 de GRUB. Cargada por la etapa 1 de GRUB.
           |               Configura el sistema y presenta el menú que
           |               permite cargar el kernel.
           |               Este archivo es obligatorio.
           |               Cuando el usuario selecciona la única opción 
           |               disponible: cargar y pasar el control al kernel
           |               que se encuentra en el directorio /boot de la imagen
           |               de disco
           |               El kernel se carga a la dirección de memoria 0x100000
           |               (1 MB)
           |                  
           kernel   <-- Archivo que contiene el código compilado del kernel.


Papel de GRUB en la carga del Kernel
------------------------------------

Al iniciar el sistema, la BIOS carga el código de GRUB almacenado en el sector
de arranque de la imagen  de disco. Cuando GRUB se instala en la imagen
de disco, el contenido del archivo stage_1 se incluye dentro del sector
de arranque. 

El código del archivo stage1 cargado en memoria recibe el control. Este código
a su vez carga en memoria la etapa 1.5 de GRUB, que ha sido copiada en la
imagen del disco a continuación del sector de arranque (por esta razón el 
archivo e2fs_state_1_5 es opcional). La etapa 1.5 de GRUB contiene el código 
para acceder a particiones de disco con formato ext2. 

El código de la etapa 1 y la etapa 1.5 de GRUB cargan la etapa 2 de GRUB
(el archivo stage_2, que es obligatorio), y luego le pasan el control.

El código de la etapa 2 lee el archivo menu.lst, que contiene
las opciones de la interfaz del menú que se presenta al usuario y
las diferentes opciones de sistemas operativos a cargar. El contenido
de este archivo se reproduce a continuación:


    default 0
    timeout 10
    color cyan/blue white/blue
    
    title Aprendiendo Sistemas Operativos
    root (hd0,0)
    kernel /boot/kernel
    

Básicamente se establece un menú en el cual la primera (y única) opción de 
arranque se define como opción por defecto. El parámetro @b timeout permite
establecer el tiempo que espera GRUB antes de cargar automáticamente la
opción de arranque definida por defecto.

Es posible definir múltiples opciones de arranque, si en (la imagen de) el disco
se tienen instalados varios sistemas operativos. (Por ejemplo, Linux y Windows).

La única opción de arranque configurada consta de tres elementos: 
- title permite especificar el texto de la opción de menú que presentará 
   GRUB
- root especifica la partición en la cual se encuentra el kernel (hd0,0).
  Esta corresponde a el disco duro primario (hd0), primera (y única) partición
  (0).
- kernel Especifica la ruta dentro de la partición en la cual se encuentra
  el kernel del sistema operativo (el código compilado de este proyecto).

Con esta configuración, al arranque del sistema se presenta el familiar menú de
GRUB con la opción "Aprendiendo Sistemas Operativos". Al seleccionar esta
opción, GRUB carga el archivo /boot/kernel de la imagen de disco
a la posición de memoria 0x100000 (1MB) y le pasa el control de la 
ejecución.
  
Muchas distribuciones actuales de Linux usan a GRUB como cargador de arranque,
mediante un sistema similar (con algunos aspectos complejos adicionales) al que
se presenta en este proyecto.


Carga del kernel por GRUB
-------------------------

El código del kernel es cargado por GRUB  a la dirección de memoria 0x100000 
(1 MB), ya que dentro del encabezado multiboot del kernel se especificó esta
como la dirección en la que se deseaba cargar el kernel (ver campo kernel_start
dentro del Encabezado Multiboot, la etiqueta multiboot_header en el archivo 
start.S). 

Debido a que GRUB es un cargador de arranque compatible con la 
especificación Multiboot, al pasar el control al kernel se tiene el siguiente
entorno de ejecución (extractado de la Especificación Multiboot, sección 3.2 
Machine state): 

- La línea de direcciones A20 se encuentra habilitada, por lo cual se tiene
  acceso a los 32 bits de los registros del procesador y a un espacio lineal
  de memoria de hasta 4 GB.
- Existe una Tabla Global de Descriptores (GDT) temporal configurada por GRUB.
  En la documentación se insiste en que el kernel deberá crear y cargar
  su propia GDT tan pronto como sea posible.
- GRUB ha obtenido información del sistema, y ha configurado en la memoria
  una estructura de datos que recopila esta información. Incluye la cantidad
  de memoria disponible, los módulos del kernel cargados (ninguno en este
  proyecto), entre otros.
- El registro EAX contiene el valor 0x2BADB002, que indica al código del kernel 
  que se usó un cargador compatible con la especificación Multiboot para 
  iniciarlo.
- El registro EBX contiene la dirección física (un apuntador de 32 bits) en la
  cual se encuentra la estructura de datos con la información recopilada por
  GRUB que puede ser usada por el kernel para conocer información del sistema
  (memoria disponible, módulos cargados, discos y dispositivos floppy, modos
  gráficos, etc).
- En el registro de control CR0 el bit PG (paginación) se encuentra en 0, por lo
  cual la paginación se encuentra deshabilitada. El bit PE (Protection Enable) 
  se encuentra en 1, por lo cual el procesador se encuentra operando en modo
  protegido de 32 bits.
- El registro de segmento de código (CS) contiene un selector que referencia 
  un descriptor de segmento válido dentro de la GDT temporal configurada por 
  GRUB. El segmento descrito se ha configurado como segmento de @b código en 
  modo plano (flat), es decir que tiene base 0 y límite 4 GB.
- Los registros de segmento de datos y pila (DS, ES, FS, GS y SS) contienen
  un selector que referencia un descriptor de segmento válido dentro de la GDT
  temporal configurada por GRUB. El segmento descrito se ha configurado como un 
  segmento de @b datos en modo plano (flat), es decir que tiene base 0 y límite
  4 GB.
- El registro apuntador al tope de la pila (ESP) debe ser configurado por el 
  kernel tan pronto como sea posible.
- El kernel debe deshabilitar las interrupciones, hasta que configure y cargue
  una tabla de descriptores de interrupción (IDT) válida. Dado a que 
  el procesador ya se encuentra en modo protegido, no se puede usar la IDT
  que la BIOS configura al inicio del sistema.
  

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
          16     | cmdline           | (presente si flags[2] = 1)
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
para mapear el kernel en memoria virtual. Luego se habilita la paginación.

Finalmente se pasa el control a la rutina cmain, definida en el archivo
kernel.c.


    call cmain /* Pasar el control a la rutina 'cmain' en el archivo kernel.c */


Ejecución del código en C del Kernel 
------------------------------------

Dentro de la función cmain se continúa con la inicialización del kernel. En esta
función se debe incluir la llamada a otras rutinas que permitirán tener un 
kernel, y (en un futuro) un sistema operativo funcional.

Dado que el código del kernel se encuentra ejecutando en modo protegido de 32 
bits, con segmentos de código y datos definidos en modo plano, es posible
acceder a la memoria de video en la Dirección lineal 0xB8000 (ya que se traduce
a la misma dirección física).

Cuando la función cmain() termina, el control se cede de nuevo al código del 
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
- David Jurgens, Help-PC Reference Library http://stanislavs.org/helppc/idx_interrupt.html
