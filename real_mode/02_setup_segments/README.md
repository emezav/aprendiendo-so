Descripción general del proyecto
================================
En este proyecto se ilustra la configuración de los registros de segmento
de código, datos y pila en modo real.

Una vez que el sector de arranque ha recibido el control de la ejecución,
debe configurar los registros de segmento de forma que pueda acceder 
correctamente a la memoria para leer o escribir datos, y también para usar
la pila.

Cuando el procesador se encuentra operando en modo real hace uso de un tipo
especial de segmentación de la memoria. Todas las direcciones lógicas
usadas en el programa cuentan con dos elementos: 

- Un selector, que almacena la dirección lineal de inicio (base) del
  segmento de memoria dividida en 16.
- Un desplazamiento, el cual se suma a la base del segmento.

La unidad de gestión de memoria toma el valor almacenado en el selector y
lo multiplica automáticamente por 16, con lo cual obtiene la base del
segmento. A este resultado le suma el desplazamiento para obtener la 
dirección lineal correspondiente. 

Dado que en modo real no es posible activar la memoria virtual, la
dirección lineal obtenida equivale a la misma dirección física en RAM.

Es necesario configurar los valores en los registros de segmento de tal forma
que la base de los segmentos de código y datos apunte a la dirección lineal
0x7C00. Esto le permite que todos los desplazamientos (las direcciones de
memoria) en el código sean relativos a donde se encuentra en memoria, y no a la
posición fija 0x7C00.

Al realizar este proceso se obtiene una ventaja adicional: El código del sector
de arranque puede ser ubicado y ejecutado en cualquier posición de memoria (no
necesariamente en 0x7C00), siempre y cuando se configuren adecuadamente los
registros de segmento de código y datos.

Los registros de segmento no se pueden modificar directamente usando
instrucciones mov, por lo cual se deberá usar estrategias diferentes para
establecer los valores dentro del registro de segmento de código y los registros
de segmento de datos.  Para lograr que el sector de arranque se pueda ejecutar
correctamente, se debe compilar y construir el archivo binario de forma que
todos sus desplazamientos (referencias a memoria de las variables y subrutinas)
sean relativos al inicio del mismo. Esto se realiza mediante las directivas
señaladas en el archivo de configuración de ld que se usa para compilar el
sector de arranque:

    
    ENTRY(start); 
    OUTPUT_FORMAT(binary);
    /* dirección relativa al inicio del bootsector*/ 
    phys = 0x0;
    SECTIONS {
       . = phys; 
       .text : { 
         *(.text)		  
         . = phys + 510;	
    
         BYTE (0x55)
         BYTE (0xAA)
       } = 0x90 
       /DISCARD/ : { 
	       *(.data)
	       *(.rodata)
	       *(.bss)
       }
    }


Configuración del registro de segmento de código
-------------------------------------------------

El registro de segmento de código CS puede ser modificado mediante una 
instrucción JMP FAR (LJMP), en la cual se especifica un selector y un
desplazamiento:

    .global start                  
    /**
    * Símbolo que marca el inicio del código del sector de arranque 
    */
    start:
    
      /* La ejecución comienza en este punto, con CS = 0x0000 e IP = 0x7C00 */
      /* CS = 0x7C0, IP = desplazamiento de entry_point */
      ljmp 0x7C0 : OFFSET entry_point
    
    entry_point:
      /* La ejecución continúa en este punto, con CS = 0x07C0 e IP = 0x0005 */

Usando este código al inicio del sector de arranque, la ejecución continúa
en la etiqueta __entry_point__, pero el registro de segmento CS contendrá
el valor de 0x07C0 con lo cual efectivamente hemos configurado la base del
segmento de código  en 0x7C00.

Configuración de los registros de segmento de datos
---------------------------------------------------

La instrucción JMP FAR sólo modifica el registro de segmento CS, por lo
cual debemos configurar al menos el registro de segmento de datos DS para
que apunte al inicio del sector de arranque en memoria.

Dado que el registro CS ya contiene 0x07C0, es posible tomar este valor y
copiarlo en el registro de datos DS, usando al registro AX como
intermediario:
    
    .global start                  
    /**
    * Símbolo que marca el inicio del código del sector de arranque 
    */
    start:
    
      /* La ejecución comienza en este punto, con CS = 0x0000 e IP = 0x7C00 */
      /* CS = 0x7C0, IP = desplazamiento de entry_point */
      ljmp 0x7C0 : OFFSET entry_point
    
    entry_point:
      /* La ejecución continúa en este punto, con CS = 0x07C0 e IP = 0x0005 */
      
      /* AX = CS */
      mov ax, cs
      /* DS = AX */
      mov ds, ax
      /* ES = AX */
      mov fs, ax
      /* FS = AX */
      mov fs, ax
      /* GS = AX */
      mov gs, ax

Con esta secuencia de instrucciones tanto CS como DS almacenarán 0x07C0,
es decir, apuntarán a segmentos de código y datos que inician en la
dirección lineal 0x07C0 * 0x10 = 0x7C00. También se pueden  configurar los
registros de segmento ES, FS y GS.

Configuración de la pila
-------------------------
Antes de continuar, también es necesario configurar adecuadamente la pila.
Como se explicó en capítulos anteriores, la pila es un elemento 
fundamental en la arquitectura x86 que se usa implícitamente para gestionar
las interrupciones y cuando se implementan e invocan subrutinas.

Por tal razón, es conveniente que al inicio del código del sector de
arranque se configure una pila en una región de memoria disponible. Como la
 pila crece hacia atrás en la memoria, podemos establecer la dirección
 0x9FC00 como el tope inicial.

Para configurar la pila se deben modificar dos registros: el registro de
segmento SS, el cual contendrá (como todos los registros de segmento)
la dirección de inicio del segmento dividida por 16, y el registro SP, que
contendrá el desplazamiento en bytes desde el inicio del segmento en el
cual se encuentra el tope de la pila.

Dado que vamos a configurar el tope de la pila en la dirección lineal
0x9FC00, debemos calcular una dirección lógica selector:offset
correspondiente. Por ejemplo, la dirección lógica 0x9000 : 0xFC00, define
que la pila tendrá como dirección de inicio 0x90000 y como tope 0xFC00
(64.512 en decimal). Esto significa que la pila tendrá un tamaño inicial
máximo de 64.512 bytes, que permitirá almacenar exactamente 32.256 valores
de 2 bytes cada uno.

Entonces, en el registro de segmento SS almacenaremos la dirección de
inicio del segmento de pila dividida por 16 (0x9000),  y en el registro SP
almacenaremos 0xFC00. Dado que la pila se usa implícitamente para gestionar
las interrupciones, es necesario deshabilitarlas temporalmente mientras se
configura la pila en su nueva ubicación.

    /* Deshabilitar temporalmente las interrupciones */
    cli
    /* AX = 0x900 */
    mov ax, 0x900
    /* SS = AX */
    mov ss, ax
    mov sp, 0xFC00
    /* Habilitar de nuevo las interrupciones */
    sti


Compilación y ejecución del proyecto
==================================

La compilación del código y la ejecución del emulador se realiza mediante la
utilidad *make*. Para ejecutar este proyecto, se debe abrir un *shell* y
ubicarse en la carpeta del proyecto. Luego se deberá ingresar uno de los
siguientes comandos, de acuerdo con el emulador que se desee usar para ejecutar
el código:
- __make qemu__: Compila el código, crea la imagen de disco floppy e Inicia el
  emulador *qemu* con la imagen de disco creada.
- __make bochs__: Similar al comando anterior, pero a cambio inicia el emulador
   *bochs*.
- __make bochsdbg__: En este caso se compila el código, se crea la imagen de
	disco floppy y se inicia el emulador *bochs* desde esta imagen de disco, con
	la interfaz gráfica del depurador activada.

También se cuenta con otros *targets* que permiten realizar otras tareas:
- __make clean__: Elimina los archivos resultado de la compilación del código y
	la imagen de disco obtenida.
- __make docs__: Genera automáticamente la documentación del proyecto en formato
	html y latex, dentro del directorio *docs*.

Puede revisar el archivo __Makefile__ que se encuentra en el directorio del
proyecto para estudiar el proceso que realiza la utilidad *make*.

Depuración paso a paso
----------------------
En el archivo bochsrc.txt se ha activado el *Magic break*, por lo cual si se
incluye la instrucción *xchg bx, bx* en cualquier parte del código, se pausará
la ejecución cuando se usa el emulador bochs con el depurador gráfico activado
(comando *make bochsdbg*).


Vea también
===========
- David Jurgens, Help-PC Reference Library http://stanislavs.org/helppc/idx_interrupt.html
