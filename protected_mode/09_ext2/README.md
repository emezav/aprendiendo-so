Descripcion general
===================

En este proyecto el kernel deja de inspeccionar estructuras `ext2` de forma
aislada y pasa a resolver rutas absolutas para leer directorios y archivos
reales del sistema de archivos.

Nota de trabajo en VS Code
--------------------------

Si se trabaja con el workspace completo del repositorio en VS Code, conviene
seleccionar la configuracion C/C++ del proyecto activo mediante
`C/C++: Select a Configuration...`.

Para este proyecto, seleccionar:

- `protected_mode/09_ext2`

Objetivo
--------

Al finalizar esta etapa, se debe poder observar que:

- el kernel resuelve nombres dentro de directorios
- sigue rutas absolutas simples dentro de `ext2`
- abre archivos regulares en solo lectura
- lee contenido real desde el sistema de archivos

Que hace el sistema en esta etapa
---------------------------------

En el estado actual del proyecto ya existen:

- la base de disco, modo de usuario y llamadas al sistema de las etapas
  anteriores
- un lector minimo de `ext2` separado del acceso crudo a sectores
- resolucion de rutas absolutas como `/boot/grub/menu.lst`
- comandos de monitor para listar directorios, leer archivos y consultar
  metadatos:
  - `ext2ls`
  - `ext2cat`
  - `ext2stat`

Que no se aborda todavia
------------------------

En esta etapa aun no aparecen:

- carga de programas `ELF` desde archivos
- mapeo de segmentos de usuario a partir de archivos reales
- una biblioteca de usuario o un shell

Que observar al probarlo
------------------------

Al compilar y ejecutar el proyecto conviene comprobar que:

- `ext2ls /boot` lista entradas reales del directorio
- `ext2ls /boot/grub` muestra archivos como `menu.lst` y `stage2`
- `ext2cat /boot/grub/menu.lst` imprime el contenido real del archivo
- `ext2stat /boot/grub/menu.lst` muestra inodo, tamano y bloques directos

Relacion con la arquitectura general
------------------------------------

Como referencia de apoyo, este proyecto se relaciona especialmente con:

- `dox/kernel/filesystem.md`
- `dox/kernel/ext2.md`

Compilacion, ejecucion y documentacion
--------------------------------------

Este proyecto es autocontenido. Se puede abrir directamente la carpeta
`protected_mode/09_ext2` en VS Code o ubicarse en ella desde una terminal
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
- las rutas de prueba de esta etapa se manejan en forma absoluta
- las trazas tecnicas de arranque y prueba quedan en `build/serial.log`
- la documentacion generada por Doxygen queda en el directorio `docs/`

Etapa previa
------------

- `08_syscalls`

Siguiente etapa
---------------

- `10_user_exec`
