# Ejemplos de programación en Modo Protegido de 32 bits

Este directorio contiene los ejemplos de programación en Modo Protegido:
- 00_enter_pmode: Paso de modo real a modo protegido.
- 00_multiboot_template: Plantilla de kernel compatible con la Especificación
  Multiboot. En esta plantilla se crea una imagen de disco con GRUB2 
  preinstalado, el cual tiene la tarea de cargar el kernel. La plantilla permite
  instalar y ejecutar el kernel en diferentes imágenes de disco:
  - Una imagen de disco para sistemas con BIOS: Esta imagen puede ser iniciada
    mediante los comandos __make bochs__ o __make qemu__.
  - Una imagen de CD. Esta imagen puede ser iniciada mediante los comandos 
    __make bochs-cd__ o __make qemu-cd__.
  - Una imagen de disco para sistemas UEFI de 32 bits: Esta imagen solo puede
    ser ejecutada por el emulador qemu, ya que bochs no cuenta con soporte para
    UEFI. El comando para crear y ejecutar la imagen de disco es 
    __make qemu-uefi32__.  
  - Una imagen de disco para sistemas UEFI de 64 bits. El comando para crear y 
    ejecutar la imagen de disco es__make qemu-uefi64__.
