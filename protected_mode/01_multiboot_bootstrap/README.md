Descripcion general
===================

Nota de trabajo en VS Code
--------------------------

Si se trabaja con el workspace completo del repositorio en VS Code, conviene
seleccionar la configuracion C/C++ del proyecto activo mediante
`C/C++: Select a Configuration...`.

Para este proyecto, seleccionar:

- `protected_mode/01_multiboot_bootstrap`

Resumen del proyecto
--------------------

`01_multiboot_bootstrap` reemplaza el arranque artesanal por un flujo de
arranque compatible con GRUB y Multiboot. En esta etapa el kernel deja de
depender de un cargador minimo propio y pasa a integrarse con un bootstrap mas
cercano al de un sistema real.

Objetivo
--------

Este proyecto tiene como objetivo mostrar como GRUB carga un kernel compatible
con Multiboot y como entrega al sistema su primer bloque de informacion de
arranque.

En particular, aqui interesa observar:

- uso de GRUB como cargador de arranque
- encabezado Multiboot del kernel
- carga del kernel en formato ELF
- recepcion de la estructura de informacion Multiboot

Que hace el sistema en esta etapa
---------------------------------

Al finalizar el arranque, el proyecto ya cuenta con:

- kernel ELF compatible con Multiboot
- imagen de disco con GRUB
- menu de arranque basico
- punto de entrada del kernel cargado por GRUB
- acceso inicial a la estructura de informacion Multiboot

Que no se aborda todavia
------------------------

En esta etapa aun no aparecen:

- paginacion propia del kernel
- layout virtual estable del kernel
- GDT e IDT completas del kernel
- infraestructura mas amplia de memoria

Que observar al probarlo
------------------------

Al compilar y ejecutar el proyecto conviene comprobar que:

- GRUB reconoce y carga el kernel correctamente
- el encabezado Multiboot es aceptado por el cargador
- el kernel recibe control en un entorno preparado por GRUB
- la informacion Multiboot queda disponible para su uso inicial

Relacion con la arquitectura general
------------------------------------

Como referencia de apoyo, este proyecto se relaciona especialmente con:

- `dox/03_entering_protected_mode.md`

Compilacion, ejecucion y documentacion
--------------------------------------

Este proyecto es autocontenido. Se puede abrir directamente la carpeta
`protected_mode/01_multiboot_bootstrap` en VS Code o ubicarse en ella desde
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

- `00_enter_pmode`

Siguiente etapa
---------------

- `02_protected_mode_setup`
