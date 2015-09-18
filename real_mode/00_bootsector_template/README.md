
Entorno de ejecución de modo real
================================

Dado que los sistemas operativos actuales configuran el procesador para operar
en *Modo Protegido* de 32 bits o *Modo de 64 bits*, no es posible ejecutar el
código de este proyecto de la misma forma como ejecutamos como cualquier otra
aplicación creada en C, Java o cualquier otro lenguaje de alto nivel. Estos
programas se compilan para ser ejecutados *dentro del entorno proporcionado por
un sistema operativo* en el caso de los programas compilados a código ejecutable
de máquina, o por el entorno proporcionado por una *máquina virtual*, en el caso
de los programas compilados a código intermedio (Java, C#, etc.).

Por lo tanto, el código debe ser ejecutado __antes__ de cargar el sistema
operativo. Por esta razón, el código se compila de forma que se obtenga un
sector de arranque válido, que es cargado automáticamente por la BIOS cuando se
inicia el computador (emulador).

El papel de la BIOS en sistemas x86
-----------------------------------
En computadores x86 equipados con BIOS, o en sistemas con UEFI en los cuales se
habilita el modo de compatibilidad, la BIOS toma el control de la ejecución del
procesador cuando se enciende o reinicia el computador. Su tarea consiste en
realizar un chequeo básico de hardware, y configurar un entorno básico en el
cual el procesador se encuentra operando en *Modo Real*. En este entorno, la
BIOS configura una Tabla de Descriptores de Interrupción y las rutinas de manejo
de interrupción que le permite gestionar de forma básica los diferentes
dispositivos de entrada y salida (de ahí su nombre, Basic Input - Output
System).

Depués de realizar la configuración del entorno, la BIOS busca un *sector* de
exactamente 512 bytes (llamado *sector de arranque*) dentro de los diferentes
dispositivos presentes (unidad floppy, disco duro, USB, red, etc.). La única
verificación que realiza la BIOS  con este sector consiste en revisar sus
últimos dos bytes, y revisar si tienen los valores 0x55 y 0xAA, respectivamente.

Si el sector leído del primer dispositivo es válido, la BIOS lo copia a la
posición de memoria 0x7C00 y *transfiere el control de la ejecución a esta
dirección de memoria*. En caso contrario, continúa con el siguiente dispositivo,
hasta encontrar un sector de arranque válido. Si no lo encuentra detiene su
ejecución.

Modo real en sistemas con UEFI
===========================

La BIOS ha sido reemplazada de forma progresiva en los sistemas x86 por un
firmware más sofisticado, llamado UEFI (Unified Extensible Firmware Interface).
Los sistemas equipados con UEFI *no tienen BIOS*, pero dependiendo del
fabricante, permiten activar un modo compatible con BIOS, que se denomina *Modo
de BIOS heredado* (Legacy BIOS Mode).  En este modo, UEFI opera de la misma
forma que una BIOS estándar, y permite iniciar un computador usando una memoria
USB o un CD / DVD.

UEFI, al igual que la BIOS, configura un entorno básico de ejecución. Sin
embargo, una de las primeras tareas que realiza es pasar a *Modo Protegido*, por
lo cual no se podrá ejecutar código compilado para Modo Real a menos que se
habilite el modo de BIOS heredado y el código se copie en una memoria USB o un
CD usando una utilidad especial.

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
proyecto para estudiar el proceso que realiza la utilidad *make*.

Descripción general del proyecto
================================
Este proyecto sirve como plantilla para crear código que se ejecutará en modo
real.  El código en ensamblador del archivo bootsect/src/bootsect.S se compilará
para crear un *sector de arranque* válido que ocupará exactamente 512 bytes, y
sus últimos dos bytes tendrán los valores 0x55 y 0xAA. Esto se logra con un
archivo de configuración (bootsect/build/link_bootsector.ld), que define la
estructura del archivo ejecutable binario obtenido.

Para ejecutar el proyecto, la utilidad *make* toma el archivo binario obtenido y
crea una *imagen de disco floppy*, la cual será cargada por los emuladores
*bochs* o *qemu*, para simular que se está iniciando el computador desde el
diskette (cuyo sector de arranque contiene el código compilado). La siguiente
figura ilustra el formato de la imagen de disco creada.

         Imagen de Floppy
         
            +--- Primer sector de la imagen de disco (sector de arranque)
            |
            v
        +-------+
        |Sector |
        |   0   |
        |       |
        +-------+

Cuando se inicia el emulador, la BIOS realizará la configuración inicial del
sistema y luego leerá el sector de arranque a la posición de memoria 0x7C00.
Luego, la BIOS transfiere el control al código dentro del sector de arranque,
mediante una instrucción *Jump* (jmp) a la posición en la cual se copió el
sector de arranque (0x7C00). El código comenzará su ejecución en Modo
Real, y podrá usar los servicios configurados de antemano por la BIOS.

La siguiente gráfica muestra el estado de la memoria del sistema cuando el
sector de arranque recibe el control por parte de la BIOS.


          Primer MegaByte de Memoria RAM luego de inicializar la BIOS
          y luego de cargar el código del sector de arranque
    
            +----------------------+ 0xFFFFF
            |                      |
            | Área de ROM          | En esta área se encuentra el código de la BIOS,
            | (BIOS, VGA, etc.)    | y se encuentra mapeada la Memoria de Video.
            |                      | 
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
            |   Memoria RAM        |  Aproximadamente 30 KB de memoria disponible
            |   disponible         |                              
            |                      |
            |                      |
            |                      | 0x7E00
     0x7DFF +----------------------+ 
            | Código del sector de |  <-- La BIOS carga el código del sector de
            | arranque (512 bytes) |      arranque aquí
            +----------------------+ 0x7C00
            |                      |
            |                      |
            |                      |
            |  Memoria RAM         | Aproximadamente 600 KB de memoria disponible
            |  disponible          |
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

Una vez que ha recibido el control de la BIOS, se puede usar los servicios
configurados por la BIOS para realizar algunas tareas, como imprimr por
pantalla, leer de teclado o leer de disco.

En esta plantilla simplemente entra en un ciclo infinito, debido a que se debe
mantener ocupado el procesador. El código de bootsect/src/bootsect.S también
incluye la siguiente instrucción:

   xchg bx, bx

Esta instrucción causa que el depurador del emulador *bochs* detenga la
ejecución del sistema y nos permita visualizar su estado, es decir, su modo de
operación, los valores almacenados en los registros, la IDT, GDT, etc.

