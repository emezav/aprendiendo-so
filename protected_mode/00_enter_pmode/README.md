Descripcion general
===================

Nota de trabajo en VS Code
--------------------------

Si se trabaja con el workspace completo del repositorio en VS Code, conviene
seleccionar la configuracion C/C++ del proyecto activo mediante
`C/C++: Select a Configuration...`.

Para este proyecto, seleccionar:

- `protected_mode/00_enter_pmode`

Resumen del proyecto
--------------------

`00_enter_pmode` muestra el cambio de modo real a modo protegido. En esta
etapa el objetivo no es construir un kernel amplio, sino hacer visible el
salto tecnico minimo que permite comenzar a ejecutar codigo de 32 bits.

Objetivo
--------

Este proyecto tiene como objetivo mostrar el mecanismo minimo necesario para
pasar de modo real a modo protegido y continuar la ejecucion en un entorno
valido de 32 bits.

En particular, aqui interesa observar:

- desactivacion de interrupciones antes del cambio de modo
- carga de una GDT minima
- activacion del bit `PE` en `CR0`
- salto largo para continuar en modo protegido

Que hace el sistema en esta etapa
---------------------------------

Al finalizar el arranque, el proyecto ya cuenta con:

- sector de arranque que carga un mini-kernel
- GDT minima para codigo y datos
- cambio real a modo protegido de 32 bits
- continuacion de la ejecucion en un segmento valido de modo protegido

Que no se aborda todavia
------------------------

En esta etapa aun no aparecen:

- paginacion
- GRUB ni Multiboot
- IDT propia del kernel
- manejo amplio de interrupciones
- kernel reutilizable

Que observar al probarlo
------------------------

Al compilar y ejecutar el proyecto conviene comprobar que:

- el sistema arranca correctamente desde el sector de boot
- la GDT minima se carga antes del cambio de modo
- el bit `PE` de `CR0` se activa en el momento esperado
- la ejecucion continua ya en modo protegido de 32 bits

Relacion con la arquitectura general
------------------------------------

Como referencia de apoyo, este proyecto se relaciona especialmente con:

- `dox/03_entering_protected_mode.md`
- `dox/01_1_gdt.md`

Compilacion, ejecucion y documentacion
--------------------------------------

Este proyecto es autocontenido. Se puede abrir directamente la carpeta
`protected_mode/00_enter_pmode` en VS Code o ubicarse en ella desde una
terminal para compilar, ejecutar y generar su documentacion.

Comandos principales:

- `make` o `mingw32-make`: compila el sector de arranque y el mini-kernel, y
  construye la imagen de disco
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

- ninguna

Siguiente etapa
---------------

- `01_multiboot_bootstrap`
