Descripcion general
===================

En este proyecto el kernel da el salto desde payloads embebidos hacia programas
de usuario cargados desde archivos reales.

Objetivo
--------

Al finalizar esta etapa, se debe poder observar que el kernel:

- abre un archivo `ELF`
- interpreta sus segmentos basicos
- los mapea en memoria de usuario
- transfiere el control al punto de entrada del programa
- atiende llamadas al sistema hechas por ese programa real

Que se aprende aqui
-------------------

Esta etapa conecta:

- sistema de archivos
- memoria virtual
- formato ejecutable
- cambio de privilegios
- primera ejecucion real de programas de usuario desde disco

Relacion con la siguiente etapa
-------------------------------

Una vez ya existe ejecucion de programas reales, la siguiente etapa puede
introducir una mini libc para que los programas de usuario dejen de depender de
ensamblador y de invocaciones crudas al kernel.
