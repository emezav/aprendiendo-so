# Ejemplos de programación en Modo Real

Este directorio contiene los siguientes ejemplos de programación en Modo Real:
- 00_bootsector_template: Plantilla de sector de arranque, el cual se
	compila e una imagen de disco floppy y se carga por la BIOS.
- 01_hello_bios: En este proyecto se ilustra el uso básico de los servicios
  de la BIOS, para interactuar con el usuario a través de la pantalla y el
  teclado.
- 02_setup_segments: En este proyecto se muestra un ejemplo de cómo se 
  configurar los registros de segmento en modo real y de cómo funciona la
  segmentación en este modo.
- 03_memory_access: El código de este proyecto muestra los mecanismos básicos
  para leer y escribir en la memoria usando registros de segmento de datos
  y registros de propósito general.
- 04_subroutines: muestra la definición y el uso de subrutinas en modo
  real.
- 05_load_kernel: En este proyecto se presenta un ejemplo en el cual junto
	con el sector de arranque se compila un mini-kernel (carpeta kernel), y
	el archivo binario obtenido se copia a partir del segundo sector de la
	imagen de disco floppy. La tarea del código en el sector de arranque
	consiste en cargar el kernel y pasarle el control de la ejecución.

