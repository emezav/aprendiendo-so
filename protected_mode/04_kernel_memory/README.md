Gestión de memoria para el kernel
================================
Este proyecto implementa el código para gestionar el espacio de memoria
disponible para el kernel. 

Desde el proyecto /protected_mode_setup/, el kernel se mapea y ejecuta para
ejecutarse en la *parte alta* de la memoria virtual. El primer MegaByte de
memoria física, junto con la memoria física ocupada por el kernel, se
mapean a partir de la dirección 0xC0000000 (3 GB). Sin embargo, hasta este
proyecto, no se cuenta con los mecanismos para asignar o liberar páginas en
la memoria virtual del kernel.

Organización de la memoria del kernel
-------------------------------------
El primer MegaByte de memoria y el kernel se mapean a la dirección lineal
(virtual) 0xC0000000. Luego, la memoria disponible corresponde a la región
entre el final del segmento de datos del kernel, y la dirección lineal  del
espacio de 16 MB reservado al final del espacio virtual para las diferentes
estructuras de datos (directorio de tablas de página, tablas de página y
demás). De esta forma, el espacio libre abarca desde el final del kernel en
memoria virtual hasta la dirección lineal 0xFF000000.

    +-------------------------+
    |_________________________| <- Directorio de tablas de página mapeado
    |                         |    recursivamente
    | (4 MB)                  |
    |                         |    0xFFC00000
    |_________________________| <- Tablas de página mapeadas recursivamente
    | (12 MB)                 |    0xFF000000
    |_________________________| <- Espacio reservado para diversos usos
    |                         | <- Fin de la memoria (virtual) disponible
    | (aprox. 1 GB)           |    
    | 1 GB -16MB -tam. kernel |  
    |   - 1MB                 |  
    |_________________________| <- Fin del kernel
    |                         |
    | Kernel (código y datos) |
    |_________________________|
    | (1 MB)                  |   0xC000000
    |_________________________| <- Primer MegaByte de RAM y kernel mapeados
    | Espacio virtual         |    a esta dirección
    | disponible para las     |
    | tareas                  |
    | (Aprox. 3 GB)           |
    |                         |
    |                         |
    |                         |
    |_________________________|

El espacio disponible (denominado kmem - kernel memory) de aproximadamente
1 GB se subdivide en regiones de páginas con un tamaño de
VIRTMEM_GRANULARITY (16 MB). Dentro de este espacio, se puede solicitar la
asignación de páginas libres (kmem_get_page() y
kmem_get_pages()), así como la asignación y el mapeo automático a
memoria física de páginas libres (kmem_allocate_page() y
kmem_allocate_pages()).

Cada región de memoria cuenta con un mapa de bits que permite gestionar las
páginas libres y asignadas dentro de ella. Para simplificar los algoritmos
de asignación de páginas, la máxima cantidad contigua de memoria virtual
que se puede asignar será entonces de VIRTMEM_GRANULARITY.

Compilación y ejecución del proyecto
==================================

La compilación del código y la ejecución del emulador se realiza mediante la
utilidad *make*. Para ejecutar este proyecto, se debe abrir un *shell* y
ubicarse en la carpeta del proyecto. Luego se deberá ingresar uno de los
siguientes comandos, de acuerdo con el emulador que se desee usar para ejecutar
el código:
- __make qemu__: Compila el código, crea la imagen de disco floppy e Inicia el
  emulador *qemu* con la imagen de disco creada.
- __make bochs__: Similar al comando anterior, pero a cambio inicia el emulador
   *bochs*.
- __make bochsdbg__: En este caso se compila el código, se crea la imagen de
    disco floppy y se inicia el emulador *bochs* desde esta imagen de disco, con
    la interfaz gráfica del depurador activada.

También se cuenta con otros *targets* que permiten realizar otras tareas:
- __make clean__: Elimina los archivos resultado de la compilación del código y
    la imagen de disco obtenida.
- __make docs__: Genera automáticamente la documentación del proyecto en formato
    html y latex, dentro del directorio *docs*.

Puede revisar el archivo __Makefile__ que se encuentra en el directorio del
proyecto para estudiar el proceso que realiza la utilidad *make* para 
compilar el código fuente y generar la imagen de disco.

Depuración paso a paso
======================
En el archivo bochsrc.txt se ha activado el *Magic break*, por lo cual si se
incluye la instrucción *xchg bx, bx* en cualquier parte del código, se pausará
la ejecución cuando se usa el emulador bochs con el depurador gráfico activado
(comando *make bochsdbg*).


Vea también
===========
- Intel® 64 and IA-32 Architectures Software Developer’s Manual Volume 3A:
  System Programming Guide, Part 1 https://software.intel.com/en-us/articles/intel-sdm
