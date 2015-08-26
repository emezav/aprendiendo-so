Entorno de Desarrollo y Ejecución
=================================

Para el desarrollo y la ejecución de los proyectos de la Serie Aprendiendo
Sistemas Operativos se requiere, además del código, los siguientes
programas:

- Editor de texto o IDE: Permite la edición de código. Un IDE permite además
  compilar y ejecutar los ejemplos de forma más ágil. En caso de no contar con
  un IDE, se puede usar una consola de comandos.
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

Enlaces externos
================

- <http://www.mingw.org> Minimalist GNU for Windows
- <http://www.gzip.org> Gzip
- <http://wiki.qemu.org/Main_Page> Qemu open source processor emulator
- <http://bochs.sourceforge.net> The Bochs IA-32 Emulator project
- <http://www.stack.nl/~dimitri/doxygen/> Doxygen
- <http://sourceforge.net/projects/e2fsimage/> Página de la utilidad e2fsimage
- <http://sourceforge.net/projects/e2fsimage/> dd for Windows
