# Módulo principal del kernel

Este módulo implementa la funcionalidad básica del paso a modo protegido de
32 bits. Recibe el control del cargador de arranque (GRUB) y se encarga de
inicializar los módulos necesarios para la ejecución del kernel.

## Dependencias
- console
- physmem
- paging

## Inicialización
La inicialización del kernel se realiza en el código de archivo start.S,
que recibe el control de GRUB. Su primera tarea consiste en usar la
información obtenida del cargador de arranque para configurar el
directorio de tablas de página y las tablas de página necesarias para
cargar el kernel en la parte alta del espacio lineal, a partir de la
dirección 3 GB (0xC0000000). En esta dirección se mapea el primer megabyte
de memoria física y el kernel.

Después de cargar el directorio de tablas de página se configura y carga la
Tabla Global de Descriptores (GDT) que usará el kernel durante toda su
ejecución.

Finalmente, el control se pasa a la subrutina cmain (kernel.c), en la cual
se deben inicializar los diferentes módulos que ofrecen las funcionalidades
básicas del kernel.

