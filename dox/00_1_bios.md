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

Inicio    | Fin     | Tamaño        | Descripción
----------| ------- | ------------- | -----------
0xA0000   | 0xAFFFF | 10000 (64 KB) | Framebuffer VGA
0xB0000   | 0xB7FFF | 8000 (32 KB)  | VGA texto, monocromático
0x0xB8000 | 0xBFFFF | 8000 (32 KB)  | VGA texto, color
0xC0000   | 0xC7FFF | 8000 (32 KB)  | BIOS de video
0xF0000   | 0xFFFFF | 10000 (64 KB) | BIOS de la board


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

- Diseño modular.
- Arquitectura y drivers independiente de la CPU.
- Ambiente de ejecución que permite ejecutar aplicaciones antes de cargar un
  sistema operativo.
- Soporte de red.

Uso básico de los servicios de la BIOS
=======================================

Para usar los servicios de la BIOS en modo real, el código debe establecer
primero los registros del procesador en unos valores específicos para cada
servicio. Luego se debe invocar una interrupción por software, por medio de
la instrucción int N (donde N es el numero de la interrupción).

A continuación se presentan los servicios mas comunes que se usan en modo
real. Para cada uno de ellos, se especifica el número de la interrupción
que se debe invocar, y los valores de los registros del procesador que
deben ser establecidos antes de invocar la interrupcion y los valores que
retorna la BIOS.

Servicios de video (int 0x10)
-------------------------------

La BIOS ofrece una serie de servicios de video por medio de la interrupción
0x10. A continuación se presentan los servicios de video más importantes:

Establecer el modo de video
---------------------------
    
        AH = 0x00
        AL = modo de video


Algunos modos de video válidos para cualquier tarjeta de video VGA son:

       AL = 0x00  Modo texto de 25 filas, 40 columnas, blanco y negro
          = 0x01  Modo texto de 25 filas, 40 columnas, 16 colores
          = 0x02  Modo texto de 25 filas, 80 columas, escala de grises
          = 0x03  Modo texto de 25 filas, 80 columnas, 16 colores
          = 0x04  Modo grafico de 320x200 pixels, 4 colores por pixel
          = 0x0D  Modo grafico de 320x200 pixels, 16 colores por pixel
          = 0x0E  Modo grafico de 640x200 pixels, 16 colores por pixel          
          = 0x11  Modo grafico de 640x480 pixels, blanco y negro
          = 0x12  Modo grafico de 640x480 pixels, 16 colores por pixel
          = 0x13  Modo grafico de 320x200 pixels, 256 colores por pixel


Estos modos dependen del tipo de tarjeta grafica (CGA, EGA, VGA). El modo
de video por defecto es Modo Texto, 25 filas, 80 columnas, 16 colores.

Leer la posición actual del cursor
----------------------------------

     ah  = 0x03


La BIOS almacena la fila actual del cursor en el registro DH, y la columna
actual en el registro DL.

Establecer la posicion del cursor
----------------------------------

        AH = 02
        DH = fila
        DL = columna


Esta rutina establece la posicion del cursor a los valores especificados.
Todas las posiciones son relativas a 0,0, que representa la esquina
superior de la pantalla. Si no se configura de otra forma, la pantalla de
video de solo texto ocupa 25 filas por 80 columnas (fila 0 a 24, columna 0
a 79).

Escribir un caracter en modo 'terminal' (modo texto)
-----------------------------------------------------

        AH = 0x0E
        AL = Caracter ascii que se desea escribir


Esta rutina imprime el caracter en la pantalla. Los caracteres Backspace
(0x08), Fin de línea (0x0A) y Retorno de Carro (0x0D) se tratan de forma
consistente. Igualmente, se actualiza la posición actual del cursor.

 Servicios de teclado(int 0x16)
 ------------------------------

La BIOS ofrece una serie de servicios de teclado por medio de la
interrupción 0x16. A continuación se presentan los servicios de teclado mas
importantes:

Leer un caracter de teclado
---------------------------

        AH = 0x00


La BIOS almacena el caracter ASCII leido en el registro AL, y el código de
escaneo (Scan Code) en el registro AH. Si se presionó una tecla de funcion
especial (shift, esc, F1, etc), AL contiene 0x00.

Leer el estado del teclado
---------------------------

        AH = 0x01


Si no existe una tecla presionada, AX toma el valor de cero. Si existe una
tecla presionada, AL contiene el código ASCII de la tecla, y AH contiene el
código de escaneo (Scan Code). Tambien si existe una tecla presionada, el
bit ZF (Zero Flag) del registro FLAGS se establece en 0.

Leer el estado de 'shift'
------------------------- 

        AH = 0x02

La BIOS establece los bits del registro AL con el siguiente formato:

        ¦7¦6¦5¦4¦3¦2¦1¦0¦  
        ¦ ¦ ¦ ¦ ¦ ¦ ¦ +---- right shift está presionado
        ¦ ¦ ¦ ¦ ¦ ¦ +----- left shift está presionado
        ¦ ¦ ¦ ¦ ¦ +------ CTRL está presionado
        ¦ ¦ ¦ ¦ +------- ALT está presionado
        ¦ ¦ ¦ +-------- Scroll Lock está activo
        ¦ ¦ +--------- Num Lock está activo
        ¦ +---------- Caps Lock está activo
        +----------- Insert está activo


Servicios de disco (int 0x13)
-----------------------------

Por medio de la interrupción 0x13, la BIOS ofrece una serie de servicios
para los discos. A continuación solo se presenta el servicio que permite
leer sectores del disco.

Leer sectores de disco
----------------------

       AH = 0x02
       AL = Numero de sectores a leer 
       CH = numero de la pista / cilindro  * ver nota
       CL = numero del sector * ver nota
       DH = numero de la cabeza 
       DL = numero del drive (0x00=floppy A:, 0x01=floppy B:, 80h=disco primario, 
                               81h=disco  secundario)
       ES:BX = Apuntador a la posición de memoria en la cual se leen los datos

 El formato del registro CX (CH:CL) es el siguiente:
    
       ¦F¦E¦D¦C¦B¦A¦9¦8¦7¦6¦5-0¦  CX
       ¦ ¦ ¦ ¦ ¦ ¦ ¦ ¦ ¦ ¦  +-----  Numero del sector (6 bits)
       ¦ ¦ ¦ ¦ ¦ ¦ ¦ ¦ +---------  2 bits mas significativos de la pista / cilindro
       +------------------------  8 bits menos significativos de la pista / cilindro


La BIOS lee el número de sectores especificados en el registro AL,
comenzando en el sector, el cilindro y la cabeza especificados en los
registros CL, CH y DH del disco especificado en DL a la posicion de memoria
apuntada por ES:BX. La BIOS entonces almacena el AH el estado de la
lectura, en AL el nÚmero de lectores que se pudieron leer, y establece el
bit CF (Carry Flag) del registro FLAGS en 0 si la lectura fue exitosa o 1
si ocurriÓ un error.

Las lecturas de disco se deberían re-intentar al menos tres veces, para
permitir que el controlador del disco se ubique en los sectores que se
desean leer. Debido a que en estos ejemplos se usa una imagen de disco, se
supone que la lectura nunca falla. En caso que la lectura falle, el código
entra en un ciclo infinito.

Ejemplos de uso de servicios de la BIOS
=======================================

Los siguientes ejemplos asumen que el código se está ejecutando en modo
real (por ejemplo este código es ejecutado por un sector de arranque
cargado por la BIOS en la dirección 0x7C00).

Imprimir el carácter '@' por pantalla
-------------------------------------

      mov ah, 0x0E /* Servicio a usar */
      mov al, 0x40 /* Caracter '@' */
      int 0x10 /* Invocar la interrupción 0x10 : servicios de video*/


Otra versión del código para imprimir un carácter por pantalla:

      mov ax, 0x0E40 /* AH = 0x0E, AL = 0x40 */
      int 0x10 /* Invocar la interrupción 0x10 : servicios de video*/

Leer un carácter del teclado
------------------------------

     mov AH, 0x00 /* Servicio de teclado : leer un caracter */
     int 0x16 /* Invocar la interrupción 0x16: servicios de teclado */


Leer el segundo sector del floppy
------------------------------------ 

Con el siguiente código se puede ller el segundo sector del floppy
(cilindro 0, pista 0, sector 2) a la dirección de memoria 0x1000

      mov AX, 0x100
      mov ES, AX /* ES = 0x100*/
      mov BX, 0x0000  /* BX = 0, por lo tanto ES:BX apunta a 0x1000 */
      
      mov AH, 0x2   /*  Servicio de disco: leer un sector */
      mov AL, 1 /* Número de sectores a leer: 1 */
      
      mov CH, 0x00 /* Pista / cilindro =  0 */
      mov DH, 0x00 /* Cabeza = 0*/
      mov CL, 0x02 /* Sector número 2 */
      
      mov DL, 0x00 /* Drive 0x00 = floppy, 0x80 = disco primario */
     
      int 0x13


Vea también

- <http://www.gnu.org/software/grub/> Página oficial de GRUB (Enlace externo)
- <http://www.uefi.org> Página oficial de UEFI (Enlace externo)
