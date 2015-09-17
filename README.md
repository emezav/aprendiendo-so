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
sistemas con procesadores x86 y los fundamentos de los sistemas 
operativos.

# Requerimientos

Para compilar y ejecutar los ejemplos es necesario contar con los siguientes
programas / utilidades:

- Editor de texto o IDE: Permite la edición de código. Un IDE permite además
  compilar y ejecutar los ejemplos de forma más ágil. En caso de no contar con
  un IDE, se puede usar un editor de texto como Vim, Emacs, Notepad, etc.
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


# Compilación y ejecución de los proyectos

Cada proyecto cuenta con un Makefile que facilita el proceso de compilación,
ejecución y documentación. Los _targets_ que se usan para compilar o
ejecutar el proyecto son:
- __all__: (Regla por defecto). Compila el código fuente y crea la imagen de
  disco.
- __clean__: Elimina los archivos resultado de la compilación y la documentación
  generada.
- __docs__: Genera la documentación del proyecto.
- __qemu__: Compila el proyecto y crea la imagen de disco si es necesario, y
  luego inicia el emulador qemu con la imagen de disco generada.
- __bochs__: Similar a la regla __qemu__, pero inicia el emulador _bochs_.
- __bochsdbg__: Igual a la anterior, pero invoca a _bochs_ iniciando el
  depurador gráfico. _Para usar esta regla, se debe tener el depurador gráfico
  habilitado en bochs_.

Para ejecutar el código de un proyecto, se usa la utilidad _Make_, la cual lee
el archivo Makefile correspondiente. El proceso es el siguiente:

- Abrir un shell (bash). Este se encuentra disponible por defecto en Linux y en
  Windows mediante MinGW/Msys.
- Navegar al directorio del proyecto que se desea ejecutar
- Ejecutar uno de los siguientes comandos:
	+ make: compila el código y crea la imagen de disco
	+ make doc: genera la documentación HTML a partir del código fuente y
	  los archivos del directorio dox/. __Se deberá abrir el archivo
	  docs/html/index.html generado para ver la documentación__.
	+ make bochs: ejecuta el emulador bochs para que arranque la imagen de
		disco.
	+ make bochsdbg: ejecuta el emulador bochs con depurador gráfico.
	+ make qemu: ejecuta el emulador qemu.
	+ make clean: borra la imagen de disco y los archivos de compilación
	  generados.

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
