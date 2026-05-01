Descripcion general
===================

En este proyecto se abre una primera puerta formal de llamadas al sistema para
programas de usuario. La etapa reutiliza la base de disco y de modo de usuario
de los proyectos anteriores, y agrega una ABI minima que permite pedir
servicios visibles al kernel mediante `int 0x80`.

Nota de trabajo en VS Code
--------------------------

Si se trabaja con el workspace completo del repositorio en VS Code, conviene
seleccionar la configuracion C/C++ del proyecto activo mediante
`C/C++: Select a Configuration...`.

Para este proyecto, seleccionar:

- `protected_mode/08_syscalls`

Objetivo
--------

Al finalizar esta etapa, se debe poder observar que:

- existe una puerta de llamadas al sistema accesible desde modo de usuario
- un programa de usuario puede pasar argumentos al kernel
- el kernel puede devolver resultados al programa que invoco la llamada
- el flujo vuelve de forma sana a modo de usuario o finaliza de manera
  controlada

Que hace el sistema en esta etapa
---------------------------------

En el estado actual del proyecto ya existen:

- lectura real de disco heredada de `06_disk_io`
- entrada y retorno controlado a modo de usuario heredados de `07_user_mode`
- una ABI minima de llamadas al sistema por `int 0x80`
- tres llamadas al sistema iniciales:
  - `write`
  - `exit`
  - `get_ticks`

Que no se aborda todavia
------------------------

En esta etapa aun no aparecen:

- lectura de archivos por nombre desde el sistema de archivos
- carga de programas `ELF` desde disco
- una biblioteca de usuario o un shell

Que observar al probarlo
------------------------

Al compilar y ejecutar el proyecto conviene comprobar que:

- el comando `syscall` invoca `write` y luego `exit`
- el mensaje escrito desde modo de usuario aparece en consola y serial
- el kernel atiende `int 0x80` sin perder la base de disco ya integrada
- las trazas muestran un retorno valido con `old_cs=0x1b`

Relacion con la arquitectura general
------------------------------------

Como referencia de apoyo, este proyecto se relaciona especialmente con:

- `dox/kernel/modo_usuario.md`
- `dox/kernel/syscalls.md`

Compilacion, ejecucion y documentacion
--------------------------------------

Este proyecto es autocontenido. Se puede abrir directamente la carpeta
`protected_mode/08_syscalls` en VS Code o ubicarse en ella desde una terminal
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

- `07_user_mode`

Siguiente etapa
---------------

- `09_ext2`
