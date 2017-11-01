Manejador de teclado
================================
Este proyecto implementa un manejador simple de teclado mediante
Entrada/Salida por interrupciones (ver kernel/include/keyboard.h y
kernel/src/keyboard.c). Durante su inicialización, la rutina
setup_keyboard() se encarga de inicializar un mapa básico de teclado e
instalar el manejador para la IRQ 1, que corresponde al controlador del
teclado.

El manejador permite procesar los eventos de teclado. Su comportamiento por
defecto consiste en determinar el estado de las teclas modificadoras
(SHIFT, ALT, CTRL, SHIFTLOCK) y actualizar una variable global con dicho
estado. Además, cuando se recibe un evento tipo BREAK con una tecla
imprimible (incluyendo ESC, BACKSPACE, TAB y ENTER), imprime el caracter
correspondiente por pantalla.

Ejercicio
=============
Se deberá implementar y probar las rutinas getchar() y gets() en
kernel/src/keyboard.c, de forma que se pueda realizar la lectura de
caracteres por teclado.

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
- 8042 Keyboard Controller http://stanislavs.org/helppc/8042.html
- Hardware Keyboard make/break codes http://stanislavs.org/helppc/make_codes.html
