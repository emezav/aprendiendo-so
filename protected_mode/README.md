Descripcion general
===================

`protected_mode/` contiene la serie de proyectos de modo protegido de este
workspace.

Como leer la serie
------------------

La progresion propuesta es acumulativa. Cada proyecto agrega una pieza nueva
encima de la base ya construida en los anteriores.

El recorrido general es:

- entrar a modo protegido
- preparar bootstrap y base del kernel
- configurar interrupciones, consola y memoria
- leer disco real
- entrar a modo de usuario
- abrir llamadas al sistema
- leer `ext2`
- cargar programas `ELF`
- construir programas de usuario

Objetivo pedagogico
-------------------

La serie busca llegar a un sistema pequeno pero visible, donde el estudiante
pueda seguir el camino completo desde el arranque del kernel hasta la ejecucion
de programas de usuario sobre un sistema de archivos real.

Etapas altas de la serie
------------------------

- base del kernel
- memoria
- disco
- modo de usuario temprano
- llamadas al sistema
- sistema de archivos
- ejecucion de programas
- shell y utilidades

Uso recomendado
---------------

Lo natural es estudiar y probar los proyectos en orden, usando cada etapa como
base conceptual y tecnica de la siguiente.
