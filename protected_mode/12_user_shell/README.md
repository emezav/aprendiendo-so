Descripcion general
===================

En este proyecto se construye una shell minima de usuario sobre el sistema de archivos
real del sistema.

Objetivo
--------

Al finalizar esta etapa, se debe poder observar que el modo de usuario permite:

- leer una linea de comandos
- interpretar entradas simples
- ejecutar programas por nombre o por path
- reutilizar la libc minima de la etapa anterior
- volver al monitor del kernel cuando el programa ejecutado termina

Que se aprende aqui
-------------------

Esta etapa conecta:

- programas de usuario
- llamadas al sistema
- sistema de archivos
- interaccion con el usuario

Estado actual
-------------

En el corte actual:

- existe un prompt minimo `ush> `
- hay comandos internos `help`, `ticks` y `exit`
- se pueden ejecutar programas como `hello.elf` o `/boot/hello.elf`
- al terminar un programa externo, el monitor del kernel vuelve a quedar usable

Todavia no existe un modelo de procesos que permita volver a la shell despues
de ejecutar un programa externo. Por ahora, cuando ese programa termina, el
control regresa al monitor del kernel.

Relacion con la siguiente etapa
-------------------------------

Una vez existe una shell minima, la siguiente etapa puede centrarse en
consolidar utilidades de usuario mas visibles.
