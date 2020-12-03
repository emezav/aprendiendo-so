# Ejemplos de programación en Modo Protegido de 32 bits

Este directorio contiene los ejemplos de programación en Modo Protegido:
- 00_enter_pmode: Paso de modo real a modo protegido. En este proyecto el sector
  de arranque y el kernel son copiados uno tras otro en una imagen de disco
  floppy sin formato. El sector de arranque lee el primer sector del kernel 
  (el segundo sector del disco) y en este sector determina el número total
  de sectores que ocupa el kernel.
  Luego el sector de arranque carga los sectores restantes del kernel, usando
  los servicios de disco de la BIOS. Finalmente el código del sector de arranque
  le transfiere el control de la ejecución al kernel.
- 00_multiboot_bootstrap: Plantilla de kernel compatible con la Especificación
  Multiboot. En esta plantilla se crea una imagen de disco con el cargador de
  arranque GRUB2 preinstalado, quien es el encargado de establecer un entorno
  básico de modo protegido, cargar el kernel en memoria y pasarle el control de
  la ejecución. 
 - 02_protected_mode_setup: Completa la inicialización del modo protegido,
  incluyendo la memoria virtual. Al finalizar la ejecución, el kernel se
  ejecuta desde su ubicación "virtual" en la memoria,  por encima de los 3 GB.
  También se inicializa la IDT, por lo cual las interrupciones no gestionadas
  causan que el kernel muestre un mensaje de advertencia de interrupción no 
  manejada.
- 03_kernel_base: Implementa la funcionalidad base del kernel. 
  Detecta y almacena la información de los dispositivos PCI presentes
  en el sistema. Esta información puede ser usada para crear y cargar los
  manejadores de dispositivo adecuados.

 Las plantillas desde el proyecto 00_multiboot_bootstrap permiten generar
 imágenes de disco y CD-ROM:
  - Imagen de disco para sistemas con BIOS: Esta imagen puede ser iniciada
    mediante los comandos __make bochs__ o __make qemu__.
  - Imagen de CD: Esta imagen puede ser iniciada mediante los comandos 
    __make bochs-cd__ o __make qemu-cd__.  

