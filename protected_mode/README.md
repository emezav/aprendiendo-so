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
  la ejecución. La plantilla permite instalar y ejecutar el kernel en diferentes
  imágenes de disco:
  - Una imagen de disco para sistemas con BIOS: Esta imagen puede ser iniciada
    mediante los comandos __make bochs__ o __make qemu__.
  - Una imagen de CD. Esta imagen puede ser iniciada mediante los comandos 
    __make bochs-cd__ o __make qemu-cd__.  
