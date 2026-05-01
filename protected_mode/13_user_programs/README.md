Descripcion general
===================

En este proyecto se consolidan las primeras utilidades de usuario visibles
encima de la shell y del sistema de archivos real.

Objetivo
--------

Al finalizar esta etapa, se debe poder ejecutar pequenos programas
como:

- `ls`
- `cat`
- `uname`
- o equivalentes pequenos segun el alcance final de la serie

Que se aprende aqui
-------------------

Esta etapa muestra como las piezas anteriores se convierten en herramientas
reales para explorar y usar el sistema.

Estado actual
-------------

En el corte actual:

- `ls.elf` lista el contenido de `/boot`
- `cat.elf` muestra el contenido de `/boot/grub/menu.lst`
- `uname.elf` imprime una identificacion minima del sistema y los ticks
- estas utilidades se ejecutan desde la shell y luego el control vuelve al
  monitor del kernel

Relacion con la siguiente etapa
-------------------------------

Cuando ya existan programas y shell funcionales, la etapa siguiente puede
reevaluar si hace falta crecer hacia un modelo de procesos mas rico.
