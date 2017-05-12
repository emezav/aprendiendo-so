Descripción general del proyecto
================================
En este proyecto se muestra el proceso necesario para pasar de modo real a
modo protegido.

El código en el sector de arranque (ver bootsect/src/bootsect.S) contiene
la lógica necesaria para cargar un mini-kernel (kerne/src/start.S) que se
compila y se copia a partir del segundo sector de la imagen floppy.

El mini-kernel recibe el control del sector de arranque, y luego ejecuta
los pasos para pasar de modo real a modo protegido de 32 bits. De acuerdo con
la documentación del manual de Intel, estos son:
1. Deshabilitar las interrupciones, usando la instrucción CLI,
2. Cargar en el registro GDTR un apuntador con la dirección (física) en la
   cual se encuentra la GDT. Esto se realiza con la instrucción LGDT.
3. Modificar el registro CR0 por medio de una instrucción MOV, para activar el 
   primer bit (PE).
4. Ejecutar una instrucción JMP o CALL para serializar el procesador. La dirección
   lógica a la cual se realiza el JMP o CALL debe estar conformada por un selector
   válido de la GDT cargada en el paso 2, y el offset dentro del segmento en el 
   cual se encuentra la próxima instrucción a ejecutar.
6. Si se ha habilitado la paginación (bit 31 de CR0), el código para las
   instrcciones MOV y JMP o CALL que establecieron el bit PE de CR0 deben provenir
   de una página mapeada a identidad, es decir, cuya dirección virtual sea
   la misma dirección física.
7. Si se va a usar una LDT, se debe ejecutar la instrucción LLDT para cargar el selector
   del segmento para la LDT en el registro LDTR.
8. Ejecutar una instrucción LTR para cargar en el registro de tareas un selector de
   segmento a la tarea inicial de modo protegido, o a un área escribible de la
   memoria que puede ser usada para almacenar la información del Segmento de Estado 
   de Tareas (TSS) en un cambio de tarea.   
9. Depués de entrar a modo protegido, se deben recargar los registros de segmento de datos 
   con selectores válidos de la GDT/ LDT configurados en pasos anteriores. También se puede
   ejecutar un JMP o CALL a una nueva tarea, lo cual automáticamente restablece los valores
   en los registros de segmento y continúa la ejecución en un nuevo segmento de código.
10. Ejecutar una instrucción LIDT para cargar el apuntador a una IDT válida para modo
   protegido.
11. Habilitar las interrupciones por medio de la instrucción STI.

En este ejemplo se ejecutan los pasos 1, 2, 3, 4 y 9. Los demás pasos se omiten, dado que no 
se activa la paginación, la multitarea ni las interrupciones.


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
----------------------
En el archivo bochsrc.txt se ha activado el *Magic break*, por lo cual si se
incluye la instrucción *xchg bx, bx* en cualquier parte del código, se pausará
la ejecución cuando se usa el emulador bochs con el depurador gráfico activado
(comando *make bochsdbg*).


Vea también
===========
- David Jurgens, Help-PC Reference Library http://stanislavs.org/helppc/idx_interrupt.html