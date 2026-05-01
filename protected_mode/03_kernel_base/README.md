Descripcion general
===================

Nota de trabajo en VS Code
--------------------------

Si se trabaja con el workspace completo del repositorio en VS Code, conviene
seleccionar la configuracion C/C++ del proyecto activo mediante
`C/C++: Select a Configuration...`.

Para este proyecto, seleccionar:

- `protected_mode/03_kernel_base`

Resumen del proyecto
--------------------

`03_kernel_base` construye la primera base estable del kernel en
`protected_mode`. En esta etapa el sistema ya arranca desde GRUB, activa
paginacion y continua la ejecucion en memoria virtual alta con estructuras
minimas propias del kernel.

Objetivo
--------

Este proyecto tiene como objetivo mostrar una base minima de kernel en modo
protegido sobre la cual puedan apoyarse las etapas siguientes.

En particular, aqui interesa observar:

- activacion inicial de paginacion
- relocalizacion del kernel a memoria virtual alta
- configuracion minima de GDT e IDT
- preparacion inicial de memoria fisica

Que hace el sistema en esta etapa
---------------------------------

Al finalizar el arranque, el proyecto ya cuenta con:

- bootstrap multiboot estable
- paginacion inicial del kernel
- doble mapeo durante la activacion de paging
- GDT propia del kernel
- IDT basica
- reprogramacion inicial del PIC
- base para gestion de memoria fisica

Que no se aborda todavia
------------------------

En esta etapa aun no aparecen:

- timer ni teclado funcionales
- monitor interactivo
- heap del kernel
- multitarea
- modo usuario
- syscalls

Que observar al probarlo
------------------------

Al compilar y ejecutar el proyecto conviene comprobar que:

- el kernel arranca correctamente desde GRUB
- la ejecucion continua luego de activar paging
- el sistema ya opera desde memoria virtual alta
- las estructuras minimas del kernel quedan inicializadas sin fallos visibles

Relacion con la arquitectura general
------------------------------------

Como referencia de apoyo, este proyecto se relaciona especialmente con:

- `dox/03_entering_protected_mode.md`
- `dox/01_1_gdt.md`
- `dox/01_2_idt_and_interrupts.md`

Compilacion, ejecucion y documentacion
--------------------------------------

Este proyecto es autocontenido. Se puede abrir directamente la carpeta
`protected_mode/03_kernel_base` en VS Code o ubicarse en ella desde una
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
- la documentacion generada por Doxygen queda en el directorio `docs/`

Etapa previa
------------

- `02_protected_mode_setup`

Siguiente etapa
---------------

- `04_irq_console`
