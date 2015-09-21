# Fundamentos de programación de sistemas de arquitectura x86

Este software está compuesto por un conjunto de proyectos de 
programación que pueden ser compilados y ejecutados de forma
independiente. Para facilitar su estudio, los proyectos se 
encuentran construidos de forma acumulativa, explorando alguna 
característica de los sistemas X86 o implementando de forma básica
alguna funcionalidad de un sistema operativo (por ejemplo la gestión de memoria
virtual, implementación de multi tarea, llamadas al sistema, 
lectura/escritura de disco, entrada por teclado, etc.).

A partir de los proyectos base se puede modificar o adaptar el código 
proporcionado para implementar otras funcionalidades o 
características, que permitan comprender con mayor profundidad los
sistemas con procesadores x86 y los fundamentos de los sistemas operativos.

# Organización de los proyectos de programación en modo real

Los proyectos de programación  en modo real se encuentran en la carpeta 
__real_mode__, y poseen la siguiente estructura de directorios:

- __build__: En esta carpeta se almacena la imagen de disco floppy que es
  cargada por los emuladores.
- __bootsect/build__: En esta carpeta se almacena el archivo binario de 512
    bytes que se obtiene al compilar el código del sector de arranque. También
    contiene los archivos de configuración que usa la utilidad _ld_ para
    compilar y crear el sector de arranque.
- __bootsect/src__: Contiene el archivo de código fuente en ensamblador del
    sector de arranque.
- __bootsect/include__: Contiene los archivos de cabecera (.h) usados en el 
  código del sector de arranque.
- __util__: Contiene scripts de utilidad que son usados cuando se crea la
  imagen de disco floppy.
- __dox__: Contiene los archivos a partir de los cuales se obtiene la
  documentación.
- __docs__: Contiene la documentación del proyecto, generada en formato html y
  latex.

En los proyectos más avanzados de modo real se encuentra también la carpeta
__kernel__, en la cual se almacena el código y los archivos de encabezado que
al compilarlos permiten obtener un mini-kernel, que será cargado por el sector
de arranque (es decir, por el código del archivo bootsect/src/bootsect.S).
La estructura de directorios dentro de la carpeta __kernel__ sigue la misma 
disposición:

- __kernel/build__: Contiene el archivo binario que se obtiene al compilar el 
  kernel.
- __kernel/src__: Contiene el código fuente del kernel.
- __kernel/include__: Contiene los archivos de cabecera que se usan en el
  código del kernel.
                          
# Organización de los proyectos de programación en modo protegido de 32 bits

Los proyectos de programación en modo protegido de 32 bits siguen una estructura
similar a los proyectos de modo real. Cada uno de ellos cuenta con las
siguientes carpetas: 

- __build__: En esta carpeta se almacena la imagen de disco que es
  cargada por los emuladores.
- __dox__: Contiene los archivos a partir de los cuales se obtiene la
  documentación.
- __docs__: Contiene la documentación del proyecto, generada en formato html y
  latex.
- __kernel/build__: Contiene el archivo binario que se obtiene al compilar el 
  kernel.
- __kernel/src__: Contiene el código fuente del kernel.
- __kernel/include__: Contiene los archivos de cabecera que se usan en el
  código del kernel.
  
  
# Compilación y ejecución de los proyectos

Cada proyecto cuenta con un Makefile que facilita el proceso de compilación,
ejecución y documentación. El proceso es el siguiente:

- Abrir un shell (bash). Este se encuentra disponible por defecto en Linux y en
  Windows mediante MinGW/Msys.
- Navegar al directorio del proyecto que se desea ejecutar
- Ejecutar el comando _make_ como se explica a continuación:
    - __make all__: (Regla por defecto). Compila el código fuente y crea la 
      imagen de disco.
    - __make clean__: Elimina los archivos resultado de la compilación y la
      documentación generada.
    - __make docs__: Genera la documentación del proyecto.
    - __make qemu__: Compila el proyecto y crea la imagen de disco si es
      necesario, y luego inicia el emulador qemu con la imagen de disco
      generada.
    - __make bochs__: Similar a la regla __qemu__, pero inicia el emulador
      _bochs_.
    - __make bochsdbg__: Igual a la anterior, pero invoca a _bochs_ iniciando
      el depurador gráfico. Para usar esta regla, se debe contar con una
      versión de _bochs_ que haya sido compilada con soporte para el depurador
      gráfico.

En los proyectos de modo protegido el kernel puede ser también ejecutado con
los siguientes comandos:
- __make qemu-cd__: Compila el proyecto y crea una imagen de CD que incluye el 
  kernel, y luego inicia el emulador qemu con la imagen de CD.
- __make bochs-cd__: Compila el proyecto y crea una imagen de CD que incluye el 
  kernel, y luego inicia el emulador bochs con la imagen de CD.
- __make qemu-uefi32__: Compila el proyecto y crea la imagen de disco para
  sistemas UEFI de 32 bits, y luego inicia el emulador qemu con la imagen de
  disco.
- __make qemu-uefi64__: Compila el proyecto y crea la imagen de disco para
  sistemas UEFI de 64 bits, y luego inicia el emulador qemu con la imagen de
  disco.

# Requerimientos

Para editar, compilar y ejecutar los proyectos es necesario contar con los 
siguientes programas / utilidades:

- Editor de texto o IDE: Permite la edición de código. Un IDE permite además
  compilar y ejecutar los ejemplos de forma más ágil. En caso de no contar con
  un IDE, se puede usar un editor de texto como Vim, Emacs, Notepad, etc. 
  Por ejemplo, se puede usar Eclipse o Netbeans.
- Compilador, Ensamblador y Linker (Requerido): Se requiere el compilador GNU de
  C (**gcc**), el ensamblador (**as**) y el Linker (**ld**), en una versión que
  permita generar archivos ELF de 32 bits. En sistemas Linux de 32 bits,
  **gcc**, **as** y **ld** se instalan por defecto o pueden ser instalados
  fácilmente. En otros sistemas es necesario (compilar o) instalar un
  _Compilador Cruzado_ (cross-compiler) que permita generar archivos ejecutables
  en formato ELF de 32 bits.
- Utilidades GNU (requeridas): se requiere además otra serie de utilidades GNU
  como **make**, **dd**, **hexdump**, **addr2line**, y **rm**. Estas utilidades
  se encuentran disponibles en Linux por defecto, y existen versiones similares
  para otros sistemas operativos. Por ejemplo en Windows estas utilidades se
  pueden instalar como parte de MinGW/Msys o Cygwin. 
- Utilidad GZIP (Requerida): permite comprimir archivos. Disponible por defecto
  en Linux, se puede instalar en otros sistemas operativos.
- Emulador de CPU o Máquina Virtual (Requerido): Un kernel de sistema operativo
  no puede ser ejecutado directamente en el hardware, si ya existe un sistema
  operativo ejecutándose. Se requiere un emulador de cpu (como bochs o qemu), o
  una máquina virtual (como VirtualBox o VMWare) para crear un _computador
  virtual_ en el cual se arranca desde la imagen de disco creada.
- Utilidades para gestión de imágenes de disco (Requerido): Debido a que en la
  mayoría de los proyectos de la serie se usa una imagen de disco que contiene
  una partición ext2, es necesario contar con la utilidad **e2fsimage**. Esta
  utilidad puede ser (compilada o) instalada en Linux y Windows.
- Utilidad para la generación de documentación (Opcional): Cada proyecto de la
  Serie Aprendiendo Sistemas Operativos permite generar su documentación en
  formato HTML o RTF gracias al software **Doxygen**. Este software se encuentra
  disponible para Linux y Windows.

# Aviso de responsabilidad / disclaimer

Este software ha sido construido a partir de la interpretación de 
los documentos documentos proporcionados por Intel y AMD, la documentación 
recopilada de otras fuentes (libros, internet, etc.), y la experiencia adquirida
en la enseñanza de la asignatura Sistemas Operativos en un entorno
universitario. Representa el trabajo personal del autor, quien hace su mejor
esfuerzo por actualizar y mejorar el código y la documentación los proyectos, y
mantenerlos libres de errores o inconsistencias.

Para lograr que el código sea sencillo de comprender, se han realizado
simplificaciones que pueden causar que el software no funcione adecuadamente en
hardware real.

El código de los proyectos puede contener código de acceso libre, lo
cual se encuentra señalado de forma explícita de ser el caso.

El software y la documentación se ofrecen sin ningún tipo de garantía implícita
o explícita, por lo cual el autor no se hace responsable por cualquier daño o
perjuicio causado por el uso parcial o total del material aquí dispuesto. 
