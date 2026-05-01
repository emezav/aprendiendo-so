Descripcion general
===================

En este proyecto el kernel conserva la lectura real de disco de la etapa
anterior y agrega una primera entrada controlada a modo de usuario. El objetivo
es comprobar que el procesador puede cambiar de nivel de privilegio y que el
kernel recupera el control de forma intencional.

Nota de trabajo en VS Code
--------------------------

Si se trabaja con el workspace completo del repositorio en VS Code, conviene
seleccionar la configuracion C/C++ del proyecto activo mediante
`C/C++: Select a Configuration...`.

Para este proyecto, seleccionar:

- `protected_mode/07_user_mode`

Objetivo
--------

Al finalizar esta etapa, se debe poder observar que:

- existen segmentos minimos para modo de usuario
- el kernel configura un TSS para cambios de privilegio
- se puede ejecutar un payload pequeno fuera del nivel de privilegio del kernel
- el control vuelve al kernel mediante una interrupcion de prueba

Que hace el sistema en esta etapa
---------------------------------

En el estado actual del proyecto ya existen:

- lectura real de disco y comandos de inspeccion heredados de `06_disk_io`
- segmentos de codigo y datos para modo de usuario
- un TSS minimo cargado con `ltr`
- una interrupcion `0x81` accesible desde modo de usuario para la primera
  prueba controlada

Que no se aborda todavia
------------------------

En esta etapa aun no aparecen:

- llamadas al sistema por `int 0x80`
- programas cargados desde el sistema de archivos
- multitarea o cambio entre varios programas de usuario

Que observar al probarlo
------------------------

Al compilar y ejecutar el proyecto conviene comprobar que:

- `ext2sb` sigue mostrando informacion valida del sistema de archivos
- el comando `usermode` entra a modo de usuario
- el retorno al kernel ocurre por `int 0x81`
- las trazas muestran `old_cs=0x1b` y `old_ss=0x23`

Relacion con la arquitectura general
------------------------------------

Como referencia de apoyo, este proyecto se relaciona especialmente con:

- `dox/kernel/memoria_virtual.md`
- `dox/kernel/modo_usuario.md`

Compilacion, ejecucion y documentacion
--------------------------------------

Este proyecto es autocontenido. Se puede abrir directamente la carpeta
`protected_mode/07_user_mode` en VS Code o ubicarse en ella desde una terminal
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

- `06_disk_io`

Siguiente etapa
---------------

- `08_syscalls`
