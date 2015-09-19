Descripción general del proyecto
================================
En este proyecto se ilustra el uso básico de los servicios de la BIOS
para imprimir por pantalla, y de los servicios de teclado para esperar por una
pulsación de tecla.

Estructura del código
----------------------
El código de este proyecto se encuentra dentro de archivo bootsect.S.
Su estructura es muy sencilla: Se hace uso de los servicios de video de la BIOS,
específicamente los servicios para imprimir un caracter, cambiar la posición de
cursor en la pantalla y para esperar por una pulsación de tecla. El proceso
realizado es el siguiente:

1. Escribir los caracteres 'H' 'e' 'l' 'l' 'o' en posición actual del cursor
2. Esperar por una pulsación de tecla.
3. Mover el cursor al inicio de la pantalla, es decir, a la esquina superior
   izquierda.
4. Limpiar la pantalla, escribiendo repetidamente el caracter espacio ' '. Dado
	que la pantalla en modo texto tiene 25 filas por 80 columnas, se deben
	imprimir 2000 espacios.
5. Mover de nuevo el cursor al inicio de la pantalla.
6. Escribir 'BIOS' al inicio de la pantalla.

Servicios de la BIOS
-------------------------------
Antes de pasar el control de la ejecución al sector de arranque (copiado a la
dirección de memoria 0x7C00), la BIOS configura una serie de rutinas de manejo
de interrupción para los diferentes dispositivos, y configura la Tabla de
Descriptores de Interrupción que se usará en modo real.

Además de configurar las rutinas para manejar las interrupciones generadas por
los diferentes dispositivos de entrada / salida, la BIOS también proporciona un
conjunto de rutinas que ofrecen *servicios*. Algunos de estos servicios se
agrupan en las siguientes categorías:
- Servicios de video: Permiten interactuar con el hardware de video, para
	escribir en la pantalla, obtener o modificar la posición del cursor, pasar a
	modo gráfico, etc. Algunas BIOS ofrecen extensiones para activar
	características avanzadas o resoluciones más altas.
- Servicios de disco: Permiten obtener información de los discos presentes en el
  sistema, leer o escribir sectores en el disco, etc.
- Servicios de teclado: Permiten interactuar con el controlador de teclado para
	realizar diferentes tareas como obtener entrada del teclado, configurar su
	modo de operación, encender o apagar los leds, etc.

La descripción de estos servicios se encuentra en la siguiente dirección URL:
http://www.stanislavs.org/helppc/


Se debe navegar a la página [Interrupt Services DOS/BIOS/EMS Mouse](http://www.stanislavs.org/helppc/idx_interrupt.html) en las siguientes categorías:

- [Servicios de video de la BIOS](http://www.stanislavs.org/helppc/int_10.html):
	Se debe revisar la documentación de los servicios INT 10,2, e INT 10, E.
- [Servicios de teclado de la
	BIOS](http://www.stanislavs.org/helppc/int_16.html): Se debe revisar la
	documentación del servicio INT 16,0.

Uso de los servicios de la BIOS
-------------------------------

Para usar los servicios ofrecidos por la BIOS, el programa primero debe
establecer ciertos valores en los registros del procesador. Entre ellos, el más
importante es el valor que se almacena en el registro AH (los 8 bits más
significativos del registro AX), el cual indica el servicio específico a usar.

Luego de establecer los valores adecuados en AH y en los demás registros
requeridos por el servicio, se invoca su ejecución usando

    int N

Esta instrucción causa una *interrupción por software*, en la cual e número *N*
representa el número de la interrupción y al mismo tiempo la categoría de los
servicios proporcionados por la BIOS en la cual se encuentra el servicio
concreto que se desea usar.

El control pasa a la rutina de manejo de la interrupción *N* configurada
de antemano por la BIOS, la cual examina el valor almacenado en AH para
determinar el servicio que debe ser ejecutado. Posteriormente, de acuerdo con el
número de servicio especificado, el código de la BIOS examina los valores
almacenados en determinados registros, los cuales pueden representar datos o
direcciones de memoria.

Al finalizar la ejecución del servicio, el control *retorna* a la instrucción
siguiente al llamado a la interrupción por software.

Ejemplo de uso de servicio de la BIOS
--------------------------------------

Por ejemplo, imprimir el caracter 'A' por pantalla usando el servicio de video
llamado *Write text in teletype mode*, se se debe especificar la siguiente
secuencia de instrucciones:

    mov ah, 0x0E /* int 0x10, ah = 0x0E Write text in teletype mode */
    mov al, 'A' /* Caracter ASCII a imprimir*/
    mov bh, 0x00 /* Página 0 en modo texto */
    mov bl, 0x00 /* Ignorado, se usa para modo gráfico */
    int 0x10 /* Invocar el servicio! */

La BIOS escribirá el caracter 'A' en la posición actual del cursor y avanzará
una posición hacia la derecha.

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

Depuración paso a paso
----------------------
En el archivo bochsrc.txt se ha activado el *Magic break*, por lo cual si se
incluye la instrucción *xchg bx, bx* en cualquier parte del código, se pausará
la ejecución cuando se usa el emulador bochs con el depurador gráfico activado
(comando *make bochsdbg*).


Vea también
===========
- David Jurgens, Help-PC Reference Library http://stanislavs.org/helppc/idx_interrupt.html
