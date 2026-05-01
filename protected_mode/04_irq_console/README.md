Descripcion general
===================

Nota de trabajo en VS Code
--------------------------

Si se trabaja con el workspace completo del repositorio en VS Code, conviene
seleccionar la configuracion C/C++ del proyecto activo mediante
`C/C++: Select a Configuration...`.

Para este proyecto, seleccionar:

- `protected_mode/04_irq_console`

Resumen del proyecto
--------------------

`04_irq_console` incorpora interrupciones de hardware observables y una
interaccion minima por consola. En esta etapa el kernel deja de ser solo una
base de arranque y pasa a reaccionar de forma visible al timer y al teclado.

Objetivo
--------

Este proyecto tiene como objetivo mostrar como integrar interrupciones utiles
de hardware en un kernel pequeno y usarlas para construir una interfaz minima
de exploracion del sistema.

En particular, aqui interesa observar:

- manejo real de `IRQ0` y `IRQ1`
- relacion entre PIC, IDT y rutinas de interrupcion
- recepcion de teclado mediante interrupciones
- uso de un monitor minimo del kernel

Que hace el sistema en esta etapa
---------------------------------

Al finalizar el arranque, el proyecto ya cuenta con:

- timer funcional por `IRQ0`
- teclado funcional por `IRQ1`
- buffer circular de teclado
- prompt interactivo `k> `
- edicion minima de linea con `BACKSPACE`
- monitor con comandos `help`, `ticks` y `clear`
- validacion observable mediante consola VGA y `build/serial.log`

Que no se aborda todavia
------------------------

En esta etapa aun no aparecen:

- heap del kernel
- multitarea
- modo usuario
- syscalls
- programas externos

Que observar al probarlo
------------------------

Al compilar y ejecutar el proyecto conviene comprobar que:

- el kernel sigue arrancando correctamente
- el timer genera ticks de forma periodica
- el teclado entrega caracteres al kernel mediante `IRQ1`
- el prompt `k> ` acepta entrada y responde a `ENTER`
- `BACKSPACE` corrige tanto el buffer logico como la salida en pantalla
- el comando `ticks` refleja actividad del timer

Relacion con la arquitectura general
------------------------------------

Como referencia de apoyo, este proyecto se relaciona especialmente con:

- `dox/01_2_idt_and_interrupts.md`
- `dox/01_3_pic.md`
- `dox/01_4_pit.md`

Compilacion, ejecucion y documentacion
--------------------------------------

Este proyecto es autocontenido. Se puede abrir directamente la carpeta
`protected_mode/04_irq_console` en VS Code o ubicarse en ella desde una
terminal para compilar, ejecutar y generar su documentacion.

Comandos principales:

- `make` o `mingw32-make`: compila el kernel y construye la imagen de disco
- `make qemu` o `mingw32-make qemu`: compila y ejecuta el proyecto en QEMU
- `make bochs` o `mingw32-make bochs`: compila y ejecuta el proyecto en Bochs
- `make bochsdbg` o `mingw32-make bochsdbg`: abre Bochs con depuracion grafica
- `make docs` o `mingw32-make docs`: genera la documentacion del proyecto con
  Doxygen
- `make clean` o `mingw32-make clean`: limpia artefactos generados

Notas practicas:

- en Windows conviene preferir `mingw32-make`
- las trazas tecnicas de arranque y prueba quedan en `build/serial.log`
- la documentacion generada por Doxygen queda en el directorio `docs/`

Etapa previa
------------

- `03_kernel_base`

Siguiente etapa
---------------

- `05_memory`
