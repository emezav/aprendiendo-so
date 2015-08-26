BIOS - Basic Input-Output System
================================

La BIOS es un componente vital dentro de los computadores personales. Es
un software que se encuentra almacenado en una ROM o integrado a la
Tarjeta Madre de los Computadores, y que se ejecuta automáticamente
cuando se enciende o reinicia el computador.

Técnicamente, cuando se enciende o reinicia el computador el registro de
segmento CS contiene el valor 0xF000 y el registro EIP contiene el valor
0xFFF0. Es decir que la primera instrucción a ejecutar se encuentra en
la dirección lineal 0xFFFF0 = 1.048.560 (Cerca del límite de 1 MB de
memoria). La instrucción que se encuentra en esta posición es un jmp
(salto) al código de la BIOS.

El código de la BIOS se encuentra mapeado en esta área de memoria
(cercana al primer MegaByte de memoria), y luego del salto anterior
recibe el control del procesador. Se debe tener en cuenta que el
procesador se encuentra operando en Modo Real, por lo cual sólo se tiene
acceso al primer MegaByte de memoria.

La primera tarea de la BIOS consiste en realizar un diagnóstico de sí
misma y de la memoria. A este diagnóstico se le conoce como POST
(Power-On Self Test). Luego realiza un diagnóstico de los dispositivos
conectados al sistema (reloj, teclado, mouse, discos, unidades de red,
tarjetas de video, etc.) y configura las rutinas básicas para manejar
las interrupciones e interactuar con los dispositivos (De ahí su nombre,
Basic Input-Output System), y configura los servicios básicos de entrada
y salida como lectura /escritura de discos, manejo del teclado y la
memoria de video, etc.

Las BIOS actuales ofrecen además una opción para entrar a su
configuración, en la cual se puede establecer diversos parámetros, de
los cuales los más importantes en la actualidad son el orden de los
dispositivos de arranque y el soporte para virtualización.

Papel de la BIOS en el inicio de un Sistema Operativo
=====================================================

Luego del chequeo inicial, la BIOS configura el primer MegaByte de
memoria con una serie de tablas y estructuras de datos necesarias para
su ejecución y para la gestión básica de los dispositivos de Entrada /
Salida. El siguiente esquema muestra la disposición del primer MegaByte
de memoria RAM cuando la BIOS comienza su ejecución.

        Primer MegaByte de Memoria RAM luego de inicializar la BIOS

            +----------------------+ 0xFFFFF
            |                      |
            | Área de ROM          | En esta área se encuentra el código de la 
            | (BIOS, VGA, etc.)    | BIOS y se encuentra mapeada la memoria de
            |                      | video.
            |                      |
            |                      | 0xA0000
    0x9FFFF +----------------------+
            | Área extendida de    |
            | datos de la BIOS     |
            | (EBDA)               |
            +----------------------+ 0x9FC00
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |   Memoria RAM        |
            |   disponible         |
            |                      |
            |                      | Aproximadamente 630 KB de espacio libre
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |                      | 0x500
       0x4FF+----------------------+
            | Área de datos de la  |
            | BIOS (BDA)           |
            |                      |
       0x3FF+----------------------+
            |                      | 
            |                      | 
            | Tabla de Descriptores| 
            | de Interrupción      | 
            | (Configurada por la  |  
            |  BIOS)               | 
         0  +----------------------+ 



La disposición del área de ROM se presenta en la siguiente tabla:

Inicio | Fin | Tamaño | Descripción
------ | --- | ------ | -----------
0xA0000 | 0xAFFFF | 10000 (64 KB) | Framebuffer VGA
0xB0000 | 0xB7FFF | 8000 (32 KB) | VGA texto, monocromático
0x0xB8000 | 0xBFFFF | 8000 (32 KB) | VGA texto, color
0xC0000 | 0xC7FFF | 8000 (32 KB) | BIOS de video
0xF0000 | 0xFFFFF | 10000 (64 KB) | BIOS de la board


Una de las estructuras de datos más importantes se encuentra al inicio
de la memoria RAM, y se llama la Tabla de Descriptores de Interrupción
(IDT). Esta estructura es un arreglo que contiene las direcciones
lógicas (direcciones en formato Segmento:desplazamiento) de las rutinas
genéricas que manejan las interrupciones de los dispositivos de
entrada/salida y las interrupciones que ofrecen servicios de entrada y
salida.

Cuando ocurre la interrupción N (invocada por software, por un
dispositivo o por una condición de error del procesador), el procesador
automáticamente almacena en la pila el valor actual de CS, IP y EFLAGS y
le pasa el control a la rutina de manejo de interrupción cuya posición
de memoria es la definida en la entrada N de esta tabla.

Las rutinas genéricas de E/S permiten realizar operaciones de básicas
(leer, escribir) de los diferentes tipos de dispositivos como el disco
floppy, el disco duro, el teclado, la memoria de video, etc. Se debe
tener en cuenta que por su caracter genérico, estas rutinas no conocen
los detalles avanzados de cada dispositivo. Por ejemplo, sólo es posible
acceder a un número limitado de sectores en el disco duro, dadas las
limitaciones del Modo Real.

El acceso a los servicios de la BIOS realiza por medio de interrupciones
por software (lanzadas con la instrucción INT), y cada interrupción
implementa un conjunto de funciones específicas. Por ejemplo, la
interrupción 0x10 permite tener acceso a una serie de servicios de video
(imprimir un caracter, mover el cursor, etc) y la interrupción 0x13
permite acceder a los servicios de disco.

Carga del Sector de Arranque
============================

Una vez que ha concluido su configuración inicial, la BIOS busca los
dispositivos configurados en los que se presume se encuentra el sistema
operativo. A estos dispositivos se les conoce como Dispositivos de
Arranque. Las BIOS actuales permiten cargar e iniciar un sistema
operativo desde diferentes dispositivos, que varían desde unidades
floppy, CD/DVD, dispositivos USB, dispositivos de red, y por supuesto
Discos Duros.

La BIOS busca y lee el primer sector (de 512 bytes) de cada dispositivo
de arranque, en el orden que tenga configurado, y verifica que los
últimos dos bytes de este sector contengan los valores 0x55 y 0xAA
(0xAA55 en little endian). Esta es la única verificación estándar que
hace la BIOS con el código del sector de arranque.

Si la BIOS no encuentra un sector de arranque válido en el primer
dispositivo, continúa con el siguiente dispositivo hasta encontrar un
sector de arranque válido. Si no puede encontrar ningún sector de
arranque válido, imprime un mensaje de error y detiene su ejecución.

El primer sector de arranque válido leído por la BIOS se carga en la
dirección de memoria 0x7C00 (31744 en decimal). El siguiente esquema
muestra la posición del código del sector de arranque en memoria.

    Primer MegaByte de Memoria RAM luego de inicializar la BIOS
    y luego de cargar el código del sector de arranque

            +----------------------+ 0xFFFFF
            |                      |
            | Área de ROM          | 
            | (BIOS, VGA, etc.)    |
            |                      |
            |                      | 0xA0000
    0x9FFFF +----------------------+
            | Área extendida de    |
            | datos de la BIOS     |
            | (EBDA)               |
            +----------------------+ 0x9FC00
            |                      |
            |                      |
            |                      |
            |                      |
            |                      |
            |   Memoria RAM        |  Aproximadamente 600 KB de memoria
            |   disponible         |                              
            |                      |
            |                      |
            |                      |
            |                      |
            |                      | 0x7E00
     0x7DFF +----------------------+ 
            | Código del sector de |   
            | arranque (512 bytes) |
            +----------------------+ 0x7C00
            |                      |
            |  Memoria RAM         | Aproximadamente 30 KB de memoria
            |  disponible          | disponible
            |                      |
            |                      | 0x500
    0x4FF   +----------------------+
            | Área de datos de la  |
            | BIOS (BDA)           |
            |                      |
    0x3FF   +----------------------+
            | Tabla de Descriptores| 
            | de Interrupción      | 
            | (Configurada por la  |  
            |  BIOS)               | 
    0       +----------------------+ 

Luego la BIOS le pasa el control de la ejecución al código del sector de
arranque, por medio de una instruccion jmp (Salto). Este salto puede
tener diferentes formatos, por ejemplo (en sintaxis Intel de
ensamblador):


ljmp 0x7C0 : 0x0000

ljmp 0x0000 : 0x7C00

Existen 4096 formas diferentes en que la BIOS puede realizar el salto,
por lo cual se debe considerar este aspecto en el código del sector de
arranque.

Además, la BIOS almacena en el registro DL un valor que indica la unidad
desde la cual se cargó el sector de arranque, el cual puede ser:

0x01 = floppy

0x80 = disco duro primario.

El código del sector de arranque deberá considerar este valor para
continuar con la carga del sistema operativo.

Carga del Sistema Operativo 
===========================

Generalmente un sector de arranque deberá contener el código para
continuar con la carga del componente central de un sistema operativo
(Kernel), lo cual implica leer la tabla de particiones del disco,
determinar la ubicación del kernel y cargarlo a memoria. Si el tamaño
del kernel es mayor a 512 KB, se deberá pasar al modo protegido, en el
cual se tiene acceso a toda la memoria RAM.

Debido a las restricciones en el tamaño del sector de arranque (ocupa
exactamente 512 bytes), este debe hacer uso de los servicios ofrecidos
por la BIOS para continuar con la carga del kernel. Estos servicios le
permiten leer los sectores de disco en los cuales se encuentra cargado
el código de inicialización del kernel.

Una vez que se ha cargado el código inicial del kernel, el sector de
arranque le pasa el control. Si el sector de arranque no ha activado el
modo protegido, una de las primeras tareas del kernel es habilitar este
modo para tener acceso a toda la memoria y a las características
avanzadas del procesador. Para este propósito se deberán implementar los pasos
requeridos para activar el modo protegido descritos en la documentación
del manual de Intel.  Como mínimo se deberán implementar los pasos 1 a 4
especificados en este manual.

El kernel continúa entonces con la carga de todo el sistema operativo,
la configuración de dispositivos y muy posiblemente el inicio de una
interfaz gráfica. Luego iniciará una serie de tareas que permitirán
iniciar sesión e interactuar con el sistema operativo.

Uso de la BIOS en los Sistemas Operativos Actuales
==================================================

Una vez cargados, los sistemas operativos modernos hacen poco o ningún
uso de la BIOS. No obstante, algunos aspectos de programación del
hardware sólo se pueden realizar por intermedio de la BIOS.

Los sistemas operativos modernos preservan el contenido del primer
MegaByte de memoria, en el cual se encuentra mapeado el código de la
BIOS. Esto les permite saltar entre el modo protegido y el modo real,
para acceder a algunos servicios que implementa la BIOS y cuya
implementación directa puede ser muy difícil.

Cargadores de Arranque
======================

La mayoría de sistemas operativos actuales dejan la responsabilidad de
su carga a programas especiales denominados Cargadores de Arranque.
Estos permiten simplificar la tarea de cargar el kernel e interactuar
con la BIOS. Los cargadores de arranque se instalan en el primer sector
del disco, reemplazando en la mayoría de las ocasiones el contenido de
este sector (otro código de arranque).

Las diferentes variantes de Linux y algunas variantes de UNIX al momento
de su instalación también instalan un cargador de arranque genérico
llamado GRUB (Grand Unified Bootlader). Este programa se instala en el
primer sector del disco duro y si existe otro sistema operativo
pre-instalado ofrece un menú al arranque que permite cargar tanto el
Linux instalado como el sistema operativo que ya se encontraba instalado
en el sistema.

Cargador de Arranque GRUB
=========================

Grub es un programa complejo, que consta de varias partes. El código
inicial de GRUB se inserta en el primer sector del disco (en el sector
de arranque) y es cargado por la BIOS al inicio del sistema en la
posición de memoria 0x7C00. Esta parte de GRUB sólo contiene el código
necesario para cargar otra porción de GRUB que se encuentra almacenada
en alguna partición del disco duro (o en otro dispositivo). La segunda
parte de GRUB contiene las rutinas específicas para gestionar el tipo de
dispositivo en el cual se encuentra almacenado el kernel (el disco duro,
un CD, una ubicación de red, etc).

Una vez que se ha cargado la segunda parte (Etapa) de GRUB, se presenta
un menú que permite elegir e iniciar alguno de los sistemas operativos
almacenados en el disco. Dependiendo de la selección del usuario, se
carga el sistema operativo requerido.

Universal Extensible Firmware Interface - UEFI
==============================================

UEFI es una especificación que define una interfaz estándar entre un firmware y
un sistema operativo, que fue concebida para superar las limitaciones de la
BIOS. A pesar que lleva más de 10 años en desarrollo, aún no ha logrado
reemplazarla de forma definitiva.  En la actualidad la última versión de UEFI es
la 2.5, aprobada en abril de 2015.  

De acuerdo co la información disponible en el sitio web oficial, la interfaz
definida por UEFI consiste en una serie de tablas de datos que contienen
información relacionada con la plataforma y una serie de llamadas a servicios de
arranque y ejecución que pueden ser usadas por el cargador de arranque y el
sistema operativo.

Algunas ventajas de de UEFI sobre las BIOS tradicionales son:

- Diseño modular
- Arquitectura y drivers independiente de la CPU
- Ambiente de ejecución que permite ejecutar aplicaciones antes de cargar un
  sistema operativo
- Soporte de red

Vea también

- <http://www.gnu.org/software/grub/> Página oficial de GRUB (Enlace externo)
- <http://www.uefi.org> Página oficial de UEFI (Enlace externo)
