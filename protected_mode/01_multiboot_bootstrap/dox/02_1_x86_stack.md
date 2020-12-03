Uso de la pila en x86
=====================
La pila es un elemento muy importante dentro de la arquitectura x86. Se puede
usar de forma explícita, y es usada de forma implícita cuando se invoca a
rutinas (instrucción call) y cuando se gestiona una interrupción.

 Organización de la Pila

Los procesadores proporcionan tres registros para manipular la pila. Estos
registros son:
- Registro de segmento SS (stack segment)
- Registro de propósito general ESP (stack pointer)
- Registro de propósito general EBP (base pointer).

La pila tiene la siguiente organización:

      Disposición de la pila en x86
      
      +-----------------+
      |                 |
      +-----------------+
      |  valor          |<--+
      +-----------------+   |
      |  valor          |<--+-----Valores almacenados en la pila
      +-----------------+   |
      |  valor          |<--+
      +-----------------+ <-- Tope de la pila. ESP = desplazamiento desde la
      |                 |                            base de la pila
      |  Espacio        |
      |  disponible en  |
      |  la pila        |
      |                 |
      |                 |
      +-----------------+ <---  Base de la pila. SS apunta a la Base de la Pila
      |                 |
      |                 |
      +-----------------+

Es importante resaltar que en los procesadores x86 la pila crece de una posición
más alta a una posición más baja en la memoria. Es decir, cada vez que se
almacena un byte o un word en la pila, ESP se decrementa y apunta a una
dirección menor en la memoria.

Operaciones sobre la pila
-------------------------

Las operaciones sobre la pila dependen del modo de ejecución del procesador, de
la siguiente forma:
- Modo real: En modo real (16 bits), la unidad mínima de almacenamiento en la pila es un word. Esto significa que si un programa intenta almacenar un byte en la pila (que es válido), el procesador insertará automáticamente un byte vacío para mantener una pila uniforme con unidades de almacenamiento de dos bytes (16 bits).
- Modo protegido: En este modo la unidad mínima de almacenamiento es un doubleword (4 bytes, 32 bits). Si se almacena un byte o un word, automáticamente se insertan los bytes necesarios para tener una pila uniforme con unidades de almacenamiento de cuatro bytes (32 bits). En el modo de compatibilidad de 64 bits también se usa esta unidad de almacenamiento.
- Modo de 64 bits: La unidad mínima de almacenamiento es un quadword (8 bytes, 64 bits). Tambien se pueden almacenar bytes, words o doublewords, ya que el procesador inserta los bytes necesarios en cada caso para tener unidades uniformes de 8 bytes (64 bits).

A continuación se presentan las instrucciones básicas para el manejo de la pila.

Instrucción push
-----------------
                 
La instrucción push almacena un valor inmediato (constante), o el valor de un registro en el tope de la pila. El apuntador al tope de la pila (ESP) se decrementa en el número de bytes almacenados.

La siguiente instrucción en modo real permite almacenar el valor del registro AX en la pila (un word):

    pushw %ax


En sintaxis Intel, simplemente se omite el sufijo 'w': 

    push ax


Esta instrucción almacena en la pila el valor de AX (2 bytes), y decrementa el valor de SP en 2. La pila lucirá así:

      +-----------------+    
      |  valor          |                                       
      +-----------------+ <-- ESP antes de la instrucción push AX
      |  valor de AX    |  push AX Coloca el valor de AX en la pila (2 bytes) 
      +-----------------+ <-- ESP = Nuevo tope de la pila luego de push AX 
      |                 |                                           
      |  Espacio        |
      |  disponible en  |
      |  la pila        |
      |                 |
      |                 |
      +-----------------+ <---  Base de la pila. SS apunta a la Base de la Pila
      |                 |
      |                 |
      +-----------------+

La instrucción push permite almacenar en la pila los valores de los registros del procesador y también valores inmediatos.

La siguiente instrucción en modo protegido de 32 bits permite almacenar el valor del registro EAX en la pila (un doubleword, 32 bits):
 
    pushl %eax


En sintaxis Intel, simplemente se omite el sufijo 'l':
 
    push eax


Esta instrucción almacena en la pila el valor de EAX (4 bytes), y decrementa el valor de ESP en 4. La pila lucirá así:

      +-----------------+    
      |  valor          |                                       
      +-----------------+ <-- ESP antes de la instrucción push EAX
      |  valor de EAX   | push EAX coloca el valor de EAX en la pila (4 bytes) 
      +-----------------+ <-- ESP = Nuevo tope de la pila luego de push EAX 
      |                 |                                           
      |  Espacio        |
      |  disponible en  |
      |  la pila        |
      |                 |
      |                 |
      +-----------------+ <---  Base de la pila. SS apunta a la Base de la Pila
      |                 |
      |                 |
      +-----------------+

Instrucción pop
----------------
                
Por su parte, la instrucción pop retira un valor de la pila (word, doubleword o quadword según el modo de operación y el sufijo de la instrucción), lo almacena en el registro destino especificado e incrementa SP (ESP o RSP según el modo de operación) en 2, 4 o 8.

Se debe tener en cuenta que luego de sacar un valor de la pila, no se puede garantizar que el valor sacado se conserve en la pila.

Por ejemplo, para sacar un valor del tope de la pila en modo protegido de 32 bits y almacenarlo en el registro EAX, se usa la siguiente instrucción:
 
    popl %eax

En sintaxis Intel:
 
    pop eax


Esta instrucción saca del tope de la pila un doubleword (cuatro bytes) y los almacena en el registro EAX, como lo muestra la siguiente figura.

      +-----------------+    
      |  valor          |                                       
      +-----------------+ <-- ESP después de pop EAX (ESP = ESP + 4)
      |  valor          | pop EAX : Este valor se almacena en EAX (4 bytes)  
      +-----------------+ <-- ESP antes de pop EAX 
      |                 |                                           
      |  Espacio        |
      |  disponible en  |
      |  la pila        |
      |                 |
      |                 |
      +-----------------+ <---  Base de la pila. SS apunta a la Base de la Pila
      |                 |
      |                 |
      +-----------------+

La instrucción pop toma el valor del tope de la pila y lo almacena en el registro de destino especificado en la misma operación. Se debe tener en cuenta ue luego de extraer un valor de la pila no se garantiza que aún siga allí.

En modo real la instrucción pop retira dos bytes de la pila, y en modo de 64 bits retira 8 bytes de la pila.

Instrucción pushf
-----------------


Esta instrucción toma el valor del registro EFLAGS y lo almacena en la pila.

      +-----------------+    
      |  valor          |                                       
      +-----------------+ <-- ESP antes de la instrucción pushf
      | valor de EFLAGS | pushf coloca el valor de EFLAGS en la pila (4 bytes) 
      +-----------------+ <-- ESP = Nuevo tope de la pila luego de pushf 
      |                 |                                           
      |  Espacio        |
      |  disponible en  |
      |  la pila        |
      |                 |
      |                 |
      +-----------------+ <---  Base de la pila. SS apunta a la Base de la Pila
      |                 |
      |                 |
      +-----------------+

En modo real solo se puede tener acceso a los 16 bits menos significativos de EFLAGS, por lo cual solo se ocupan 2 bytes en la pila y SP se decrementa en 2. En modo de 64 bits se ocupan 8 bytes (64 bits) para el registro RFLAGS.

Instrucción popf
----------------


Esta instrucción toma el valor almacenado en el tope de la pila y lo almacena en el registro EFLAGS (32 bits), los 16 bits menos significativos de EFLAGS en modo real y RFLAGS en modo de 64 bits.

      +-----------------+    
      |  valor          |                                       
      +-----------------+ <-- ESP después de popf (ESP = ESP + 4)
      |  valor          | popf : Este valor se almacena en EFLAGS (4 bytes)  
      +-----------------+ <-- ESP antes de popf 
      |                 |                                           
      |  Espacio        |
      |  disponible en  |
      |  la pila        |
      |                 |
      |                 |
      +-----------------+ <---  Base de la pila. SS apunta a la Base de la Pila
      |                 |
      |                 |
      +-----------------+

Instrucción pusha
--------------------


Esta instrucción almacena el valor de los registros de propósito general en la pila. De acuerdo con el modo de operación del procesador, se almacenarán los registros en el siguiente orden:
- En modo protegido de 32 bits, se almacenan EAX, ECX, EDX, EBX, valor de ESP antes de pusha, EBP, ESI y EDI.
- En modo real (16 bits), se almacenan AX, CX, DX, BX, valor de SP antes de pusha, BP, SI y DI.
- En modo de 64 bits, se almacenan RAX, RCX, RDX, RBX, valor de RSP antes de pusha, RBP, RSI y RDI.

Así, en modo protegido de 32 bits cada valor almacenado en la pila tomará cuatro bytes. En modo real, tomará dos bytes y en modo de 64 bits cada valor tomará 8 bytes.

A continuación se presenta un diagrama del funcionamiento de la instrucción pusha en modo protegido de 32 bits.

      +--------------------+    
      |  valor             |                                       
      +--------------------+ <-- ESP antes de pusha
      |  valor de EAX      |     |
      +--------------------+     |
      |  valor de ECX      |     |
      +--------------------+     |
      |  valor de EDX      |     |
      +--------------------+     |
      |  valor de EBX      |     |
      +--------------------+     |
      |  ESP antes de pusha| <---+
      +--------------------+
      |  valor de EBP      |  
      +--------------------+
      |  valor de ESI      |  
      +--------------------+
      |  valor de EDI      |  
      +--------------------+ <-- ESP después de pusha (ESP = ESP - 32) este es 
      |                    |     el nuevo tope de la pila.
      |  Espacio           |
      |  disponible en     |
      |  la pila           |
      |                    |
      |                    |
      +--------------------+ <---  Base de la pila. SS apunta a la Base de la Pila
      |                    |
      |                    |
      +--------------------+

Instrucción popa
-----------------


Esta instrucción extrae de la pila ocho valores, y los almacena en los registros de propósito general, en el siguiente orden (inverso al orden de pusha):

    EDI, ESI, EBP, ESP*, EBX, EDX, ECX, EAX

El valor de ESP sacado de la pila se descarta.

      +--------------------+    
      |  valor             |                                       
      +--------------------+ <-- ESP después de popa ( ESP = ESP + 32)
      |  valor             | ---> EAX
      +--------------------+      
      |  valor             | ---> ECX
      +--------------------+      
      |  valor             | ---> EDX
      +--------------------+      
      |  valor             | ---> EBX
      +--------------------+      
      |  valor             |  (este valor se descarta)
      +--------------------+
      |  valor             | ---> EBP
      +--------------------+
      |  valor             | ---> ESI
      +--------------------+
      |  valor             | ---> EDI
      +--------------------+ <-- ESP andtes de pusha 
      |                    |    
      |  Espacio           |
      |  disponible en     |
      |  la pila           |
      |                    |
      |                    |
      +--------------------+ <---  Base de la pila. SS apunta a la Base de la Pila
      |                    |
      |                    |
      +--------------------+

