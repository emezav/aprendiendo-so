Descripcion general
===================

En este proyecto el kernel da su primer paso real hacia el almacenamiento del
sistema: leer datos desde la imagen de disco.

Objetivo
--------

Al finalizar esta etapa, se debe poder observar que el kernel:

- detecta un disco ATA valido
- lee sectores por LBA
- reconoce el superblock de `ext2`
- ubica el inodo y el directorio raiz

Que se aprende aqui
-------------------

Esta etapa sirve para introducir:

- acceso ATA PIO basico
- lectura cruda de sectores
- relacion entre particion, filesystem y estructuras internas de `ext2`

Que se puede probar
-------------------

Las pruebas visibles de esta etapa incluyen comandos como:

- `ata`
- `readlba`
- `ext2sb`
- `ext2root`
- `ext2lsroot`

Con ellos se puede comprobar que el kernel ya no solo imprime cosas en
consola, sino que tambien empieza a entender estructuras reales del disco.

Relacion con la siguiente etapa
-------------------------------

Una vez el kernel puede leer el disco y seguir estructuras basicas de `ext2`,
la siguiente etapa puede concentrarse en abrir el modo de usuario sobre una
base mas realista.
