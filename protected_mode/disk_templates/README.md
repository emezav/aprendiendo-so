# Plantillas de disco

Este directorio contiene las siguientes plantillas de disco, que se usan en los
diferentes proyectos de programación en Modo Protegido:

- __disk_template.gz__: Plantilla de disco duro para sistemas con BIOS que
  contiene una partición de 10 MB formateada con el sistema de archivos EXT2.
  En esta imagen de disco se tiene el cargador de arranque GRUB2 preinstalado, y
  se copia el archivo del kernel obtenido en el proyecto. Al iniciar el emulador
  con esta imagen como disco duro, GRUB2 carga automáticamente el kernel.
  Esta plantilla se usa cuando se ejecutan los comandos __make bochs__ y
  __make qemu__.
- __cd_template.tar.gz__: Plantilla para crear una imagen de CD para sistemas 
  con BIOS. Esta plantilla se usa cuando se ejecutan los comandos
  __make bochs-cd__ y __make qemu-cd__.
- __uefi32_disk_template.gz__: Plantilla de disco duro para sistemas UEFI de 32 
  bits que contiene dos particiones: una partición de sistema UEFI de 32 MB en 
  la cual se encuentra preinstalada la versión UEFI de GRUB2, y una partición
  ext2 en la cual se copia el kernel.  Al iniciar el emulador qemu con esta 
  imagen como disco duro, GRUB2 carga automáticamente el kernel.
  Esta plantilla se usa cuando se ejecuta el comando __make qemu-uefi32__.
- __uefi64_disk_template.gz__: Plantilla de disco duro para sistemas UEFI de 64
  bits que contiene dos particiones, una partición de sistema UEFI de 32 MB en 
  la cual se encuentra preinstalada la versión UEFI de 64 bits de GRUB, y una
  partición ext2 en la cual se copia el kernel compilado.  Al iniciar el 
  emulador qemu con esta imagen como disco duro, GRUB2 carga automáticamente el 
  kernel. Esta plantilla se usa cuando se ejecuta el comando 
  __make qemu-uefi64__.
- __grub_efi.tar.gz__: Contiene una imagen de GRUB para crear una USB que puede
  ser usada como medio de arranque en sistemas UEFI de 32 y 64 bits.
