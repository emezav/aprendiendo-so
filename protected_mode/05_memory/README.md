Descripcion general
===================

Nota de trabajo en VS Code
--------------------------

Si se trabaja con el workspace completo del repositorio en VS Code, conviene
seleccionar la configuracion C/C++ del proyecto activo mediante
`C/C++: Select a Configuration...`.

Para este proyecto, seleccionar:

- `protected_mode/05_memory`

Resumen del proyecto
--------------------

`05_memory` se centra en la base de memoria del kernel. En esta etapa el
objetivo ya no es ampliar la interaccion por consola, sino entender y validar
las piezas de memoria necesarias para seguir avanzando hacia disco real y,
despues, hacia modo usuario.

Objetivo
--------

Este proyecto tiene como objetivo consolidar la gestion basica de memoria del
kernel y dejarla lista para etapas posteriores.

En particular, aqui interesa observar:

- asignacion de marcos fisicos
- mapeo dinamico de paginas del kernel
- uso de paginas virtuales del kernel
- limites y convenciones necesarias antes de introducir memoria de usuario

Que hace el sistema en esta etapa
---------------------------------

En el estado actual del proyecto ya existen:

- gestor de memoria fisica por bitmap en `kernel/src/physmem.c`
- mapeo y desmapeo dinamico de paginas en `kernel/src/paging.c`
- asignacion de paginas del kernel en `kernel/src/kmem.c`
- arranque observable por consola, timer y teclado

Que no se aborda todavia
------------------------

En esta etapa aun no aparecen:

- lectura real desde disco
- entrada a modo de usuario
- syscalls para programas de usuario
- espacios de direccion separados con una ABI definida
- carga de programas reales desde disco y `ext2`

Que observar al probarlo
------------------------

Al compilar y ejecutar el proyecto conviene comprobar que:

- el kernel sigue arrancando sin perder estabilidad respecto a `04_irq_console`
- las rutinas de memoria no rompen el flujo normal de arranque
- el monitor y las IRQ siguen funcionando mientras se consolida la base de
  memoria
- las futuras mediciones o diagnosticos de memoria pueden observarse de forma
  clara por consola o por serial

Relacion con la arquitectura general
------------------------------------

Como referencia de apoyo, este proyecto se relaciona especialmente con:

- `dox/01_5_physical_memory.md`
- `dox/01_6_virtual_memory.md`
- `dox/01_7_kernel_heap.md`

Compilacion, ejecucion y documentacion
--------------------------------------

Este proyecto es autocontenido. Se puede abrir directamente la carpeta
`protected_mode/05_memory` en VS Code o ubicarse en ella desde una terminal
para compilar, ejecutar y generar su documentacion.

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

- `04_irq_console`

Siguiente etapa
---------------

- `06_disk_io`
