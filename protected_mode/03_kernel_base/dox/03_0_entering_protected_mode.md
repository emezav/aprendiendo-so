Entrar a Modo Protegido
=======================

El manual de intel "Intel Architecture Software Developer's Manual , Volume 3:
System Programming" (Codigo 243192), en su sección 8.8.1 Switching to Protected
Mode, especifica el procedimiento requerido para pasar de modo real a modo
protegido. Este procedimiento asegura compatibilidad con todos los procesadores
Intel de arquitectura x86. Los pasos son:

1. Deshabilitar las interrupciones por medio de la instruccion CLI
   No incluido en el manual de Intel: Por razones históricas, cuando el
   procesador se encuentra en modo real, su línea de direcciones 20 (A20 Gate)
   se encuentra deshabilitada, lo cual causa que cualquier dirección de memoria
   mayor a 2^20 (1MB), sea truncada al limite de 1 MB. Debido a que en modo
   protegido es necesario usar los 32 bits del bus de direcciones para
   referenciar hasta 2^32 = 4GB de memoria, es necesario habilitar la linea de
   direcciones A20. Esto se logra por medio del controlador 8042 (teclado), el
   cual se puede encontrar fisicamente en la board o integrado dentro de su
   funcionalidad.
2. Ejecutar LGDT para cargar una tabla global de descriptores (GDT) válida.
3. Ejecutar una instruccion MOV para establecer el bit 0 del registro de control
   CR0 (PE = Protection Enable).
4. Inmediatamente después de establecer el bit PE en CR0, se debe realizar un
   jmp/call para limpiar la cola de pre-fetch. Si se habilitó paginacion (bit PG
   = Page Enable), las instrucciones MOV y JMP/CALL deberan estar almacenadas en
   una página cuya dirección virtual sea idéntica a la dirección fisica.
5. Si se va a utilizar un LDT, se debe cargar por medio de la instrucción LLDT.
6. Ejecutar una instrucción LTR para cargar el Task Register con el selector de
   la tarea inicial, o de un área de memoria escribible que pueda ser utilizada
   para almacenar información de TSS en un Task Switch (cambio de contexto).
7. Luego de entrar en modo protegido, los registros de segmento (DS, ES, FS, GS
   y SS) aún contienen los valores que tenían en modo real (el jmp del paso 4
   sólo modifica CS). Se deben cargar selectores validos en estos registros, o
   el selector nulo (0x0).
8. Ejecutar LIDT para cargar una Tabla de Descriptores de Interrupcion válida.
   La IDT deberá contener entradas válidas al menos para las 32 primeras
   entradas, que corresponden a las excepciones de la arquitectura x86.
9. Habilitar las interrupciones por medio de la instrucción STI.

Cuando se usa un cargador compatible con la Especificación Multiboot para cargar
el kernel (como GRUB), se cuenta con las siguientes facilidades:

- La línea de direcciones A20 ya se encuentra activada.
- Ya se ha configurado una GDT temporal. La especificación Multiboot insiste en
	que se deberá configurar y cargar una GDT propia del kernel tan pronto como
	sea necesario.
- El kernel debe configurar una pila (los registros SS y ESP) tan pronto como
	sea posible.
