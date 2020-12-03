# Módulos del kernel

Esta carpeta contiene los módulos que se adicionan al kernel.
La estructura de cada módulo debe coincidir con la estructura base del
kernel en los siguientes sub-directorios:
- include: Contiene los archivos de cabecera (.h).
- src: Contiene los archivos de implementación (.c).

Cada módulo debe ser copiado al subdirectorio kernel/ del proyecto en el
cual se va a usar. Dado que en el Makefile se detecta automáticamente los
archivos .c existentes en el subdirectorio kernel/src, no es necesario
realizar ninguna modificación adicional.

## Dependencias con otros módulos del kernel

Dentro del archivo README de cada módulo (cuyo nombre deberá ser único), se
especifican los módulos de los cuales depende y que deben ser inicializados
con anterioridad.

Los módulos deben ser inicializados en el orden correcto dentro del
controlador principal del kernel (la función cmain en el archivo
kernel/src/kernel.c).

