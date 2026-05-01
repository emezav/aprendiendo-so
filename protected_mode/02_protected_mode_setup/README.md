Descripcion general
===================

Nota de trabajo en VS Code
--------------------------

Si se trabaja con el workspace completo del repositorio en VS Code, conviene
seleccionar la configuracion C/C++ del proyecto activo mediante
`C/C++: Select a Configuration...`.

Para este proyecto, seleccionar:

- `protected_mode/02_protected_mode_setup`

Resumen del proyecto
--------------------

`02_protected_mode_setup` consolida el entorno inicial del kernel despues del
arranque por GRUB. En esta etapa se organizan las primeras piezas propias del
kernel en modo protegido antes de abrir una base mas estable en `03`.

Objetivo
--------

Este proyecto tiene como objetivo mostrar la transicion entre el entorno
temporal del cargador y un entorno de kernel mas controlado dentro de modo
protegido.

En particular, aqui interesa observar:

- reorganizacion del entorno inicial del kernel
- preparacion de estructuras propias del kernel
- diferencia entre el estado entregado por GRUB y el estado que el kernel
  quiere mantener

Que hace el sistema en esta etapa
---------------------------------

Al finalizar el arranque, el proyecto ya cuenta con:

- continuidad del bootstrap multiboot
- organizacion inicial del codigo del kernel en modo protegido
- base mas clara para cargar estructuras propias del kernel

Que no se aborda todavia
------------------------

En esta etapa aun no aparecen:

- paginacion alta del kernel
- IDT funcional completa
- consola interactiva
- multitarea
- modo usuario

Que observar al probarlo
------------------------

Al compilar y ejecutar el proyecto conviene comprobar que:

- el kernel arranca correctamente bajo GRUB
- la ejecucion en modo protegido sigue un flujo mas controlado
- el sistema ya prepara un entorno propio del kernel
- esta base puede servir como puente hacia `03_kernel_base`

Relacion con la arquitectura general
------------------------------------

Como referencia de apoyo, este proyecto se relaciona especialmente con:

- `dox/03_entering_protected_mode.md`
- `dox/01_1_gdt.md`

Compilacion, ejecucion y documentacion
--------------------------------------

Este proyecto es autocontenido. Se puede abrir directamente la carpeta
`protected_mode/02_protected_mode_setup` en VS Code o ubicarse en ella desde
una terminal para compilar, ejecutar y generar su documentacion.

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
- la documentacion generada por Doxygen queda en el directorio `docs/`

Etapa previa
------------

- `01_multiboot_bootstrap`

Siguiente etapa
---------------

- `03_kernel_base`
