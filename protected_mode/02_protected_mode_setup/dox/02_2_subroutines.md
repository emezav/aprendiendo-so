Subrutinas
===========

En los lenguajes de alto nivel, además de bifurcaciones y ciclos, también se usan las rutinas como mecanismos para organizar la funcionalidad de un programa.

Para implementar rutinas, el procesador incluye entre otras las instrucciones call (invocar una rutina) y ret (retornar de la rutina).

Una rutina consta de varios componentes, entre los cuales sobresalen:
1.Nombre de la rutina: Símbolo dentro de la sección de texto que indica el inicio de la rutina.
2.Parámetros de entrada: Se utiliza la pila para pasar los parámetros a las funciones.
3.Dirección de retorno: Dirección a la cual se debe retornar una vez que se ejecuta la rutina

Por ejemplo, para definir una rutina llamada nombre_rutina, el código sería el siguiente:

    nombre_rutina:/* Inicio de la rutina */
       /* Instrucciones de la rutina*/
       ...
       ...
       ...
      ret /* Fin de la rutina  (retornar) */

Es necesario notar que la definición de una etiqueta no necesariamente implica la definición de una rutina. El concepto de "Rutina" lo da el uso que se haga de la etiqueta. Si para saltar a una etiqueta se usa la instrucción de salto incondicional (jmp) o alguna instrucción de salto condicional (j..), esta no es una rutina. Si por el contrario, para saltar a una etiqueta se usa la instrucción call (ver explicación más adelante), y después de esa etiqueta existe una instrucción ret a la cual se llega sin importar la lógica de programación, entonces la etiqueta sí puede ser considerada una "Rutina".

Invocación a Rutinas
--------------------

La llamada a una rutina se realiza por medio de la instrucción __call__ (en sintaxis AT&T e Intel), especificando la etiqueta (el nombre de la rutina) definido en ensamblador:
call nombre_rutina


De esta forma, se estará invocando a la rutina nombre_rutina, sin pasarle parámetros.

También es posible, aunque poco común, realizar llamadas a rutinas que se encuentran en otros segmentos de memoria. En este caso se utiliza la instrucción lcall.

 Parámetros de entrada de las rutinas

Si se desean pasar parámetros a una rutina, éstos se deben almacenar en la pila mediante la instrucción push, en el orden inverso en el que se van a utilizar en la rutina antes de ejecutar la instrucción call.

Por ejemplo, para invocar a una rutina y pasarle n parámetros (parámetro 1, parámetro 2, .. , parámetro n), primero se deben insertar los parámetros en orden inverso en la pila (del último al primero) antes de la instrucción call:

    push parametro n
    push parametro n-1
    ...
    push parametro 2
    push parametro 1


La cantidad de bytes que ocupa cada parámetro en la pila depende del modo de operación del procesador. En modo real, cada parámetro ocupa dos bytes (16 bits). En modo protegido, cuatro bytes (32 bits) y en modo de 64 bits cada parámetro ocupa ocho bytes.

Luego se utiliza la instrucción call, especificando el símbolo (la etiqueta) de la rutina que se desea ejecutar:

    call nombre_rutina


La instrucción call almacena automáticamente en la pila dirección de memoria de la próxima instrucción a ejecutar luego del call (la dirección de retorno), y establece el registro EIP (instruction pointer) al desplazamiento en el segmento de código en la cual se encuentra definido el símbolo con el nombre de la rutina.

De esta forma, en el momento de llamar a una rutina, la pila se encuentra así:

      +--------------------+    
      |  valor             |                                       
      +--------------------+ <- ESP antes de insertar los parámetros y de
      | Parámetro N        |    invocar la rutina (call)
      +--------------------+     
      | Parámetro N-1      |     
      +--------------------+
      | .....              |  
      +--------------------+
      | Parámetro 2        |  
      +--------------------+
      | Parámetro 1        |  
      +--------------------+
      | EIP                | -> Dirección de la siguiente instrucción a ejecutar
      | (Dir. de retorno)  |    al retornar de la rutina (dirección de retorno)
      +--------------------+ <-- ESP después de call                           
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

Retorno de una rutina
---------------------

Cuando dentro de una rutina se busque retornar la ejecución al punto en el cual fue invocada, se debe usar la instrucción ret. Esta instrucción saca del tope de la pila la dirección de retorno, y establece el registro EIP con este valor. Se debe garantizar que el tope de la pila contiene una dirección de retorno válida, o de lo contrario el procesador continuará su ejecución en otro punto e incluso puede llegar a causar una excepción.

si se insertan valores en la pila dentro de la rutina, se deben extraer antes de ejecutar la instrucción ret.

Valor de retorno de las rutinas
--------------------------------

Es importante tener en cuenta que la instrucción ret difiere un poco de la instrucción return de lenguajes de alto nivel, en la cual se puede retornar un valor. En ensamblador, el valor de retorno por convención se almacena siempre en el registro AL, AX, EAX o RAX de acuerdo con el modo de operación del procesador. Así, una de las últimas instrucciones dentro de la rutina antes de ret deberá almacenar el valor de retorno en el registro EAX.

El siguiente diagrama ilustra el funcionamiento de la instrucción ret.

Antes de ejecutar la instrucción ret:

          EAX
      +--------------------+    
      |  valor de retorno  |                                       
      +--------------------+
      

      +--------------------+    
      |  valor             |                                       
      +--------------------+ 
      | Parámetro N        |   
      +--------------------+     
      | Parámetro N-1      |     
      +--------------------+
      | .....              |  
      +--------------------+
      | Parámetro 2        |  
      +--------------------+
      | Parámetro 1        |  
      +--------------------+
      | EIP                | 
      | (Dir. de retorno)  |   
      +--------------------+ <-- ESP                         
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


Después de ejecutar la instrucción ret:

          EAX
      +--------------------+    
      |  valor de retorno  |                                       
      +--------------------+
      

      +--------------------+    
      |  valor             |                                       
      +--------------------+ 
      | Parámetro N        |   
      +--------------------+     
      | Parámetro N-1      |     
      +--------------------+
      | .....              |  
      +--------------------+
      | Parámetro 2        |  
      +--------------------+
      | Parámetro 1        |  
      +--------------------+ <-- ESP
      | EIP                | 
      | (Dir. de retorno)  |  --> EIP La ejecución continúa en esta dirección  
      +--------------------+                          
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

Dado que al retornar de la rutina los parámetros aún se encuentran en la pila, es necesario avanzar ESP para que apunte a la posición de memoria en la cual se encontraba antes de insertar los parámetros. Para lograr este propósito se adiciona un número de bytes a ESP: 
add esp, N


Donde N corresponde al número de bytes que se almacenaron en la pila como parámetros:
- En modo real cada parámetro ocupa dos bytes en la pila, por lo cual se deberá sumar 2 * el número de parámetros a SP.
- En modo protegido cada parámetro ocupa cuatro bytes en la pila, por lo que se deberá sumar 4 * el número de parámetros a ESP.
- En modo de 64 bits se deberá sumar 8 * el número de parámetros a RSP.

De forma general, el formato para invocar una rutina que recibe N parámetros es el siguiente:

    push parametroN
    push parametroN-1
    ...
    push parametro2
    push parametro1
    call nombre_rutina
    add esp, N


Ejemplo de implementación de Rutinas en Ensamblador
---------------------------------------------------

A continuación se muestra la implementación general de una rutina en lenguaje ensamblador. Dentro de la rutina se crea un "marco de pila", necesario para manejar correctamente las variables que fueron pasadas como parámetro en la pila y las variables locales.

El concepto de "marco de pila" se explicará tomando como base la plantilla de rutina en modo real. En este modo se usan los registros SP, BP e IP.

En los demás modos de operación del procesador el marco de pila funciona en forma similar, pero se deben expandir los registros a sus equivalentes en 32 y 64 bits, y se deberá sumar el número de bytes de acuerdo con el tamaño de los registros.

En sintaxis AT&T:
    
    nombre_rutina:
       pushw %bp   /*Almacenar %bp en la pila*/
       movw %sp, %bp  /*Establecer %bp con el valor de %sp*/
        /*Ya se ha creado un marco de pila*/
       ...
       (instrucciones de la rutina)
       ...
    
        /*Cerrar el marco de pila:*/
       movw %bp, %sp  /*Mover %bp a %sp*/
       popw %bp/*Recuperar el valor original de %bp */
    
       ret /* Retornar de la rutina */

En sintaxis Intel:

    nombre_rutina:
       push bp   /*Almacenar bp en la pila*/
       mov bp, sp  /*Establecer bp con el valor de sp*/
        /*Ya se ha creado un marco de pila*/
       ...
       (instrucciones de la rutina)
       ...
    
        /*Cerrar el marco de pila:*/
       mov sp, bp  /*Mover bp a sp*/
       pop bp/*Recuperar el valor original de %bp */
    
       ret  /* Retornar de la rutina*/

Explicación de la plantilla de rutina
----------------------------------------

En esta explicación se supone que el código ha insertado los parámetros en la pila e invocó la instrucción call para ejecutar la rutina.

Con la instrucción

    pushw %bp   /* Sintaxis AT&T */

ó 

    push bp    /* Sintaxis Intel */


La pila queda dispuesta de la siguiente forma:

      +--------------------+    
      |  valor             |                                       
      +--------------------+ 
      | Parámetro N        |   
      +--------------------+     
      | Parámetro N-1      |     
      +--------------------+
      | .....              |  
      +--------------------+
      | Parámetro 2        |  
      +--------------------+
      | Parámetro 1        |  
      +--------------------+
      |  IP                | 
      | (Dir. de retorno)  |   
      +--------------------+
      | Valor de  BP       | Se almacena el valor de BP en la pila
      +--------------------+ <-- SP                        
      |                    |                              
      |  Espacio           |    
      |  disponible en     |
      |  la pila           |
      |                    |
      |                    |
      +--------------------+ <---  SS apunta a la base de la pila 
      |                    |
      |                    |
      +--------------------+


Observe que SP apunta ahora a la posición de memoria en la pila en la cual se almacenó el valor que tenía BP originalmente. Esto permite modificar BP, y recuperar su valor original luego de terminadas las instrucciones de la rutina y antes de retornar al punto desde el cual se invocó la rutina.

La instrucción 

    movw %sp, %bp  /* Sintaxis AT&T */

ó
 
    mov bp, sp /* Sintaxis Intel */


Establece a EBP con el mismo valor de SP, con lo cual BP apunta a la misma dirección de memoria a la cual apunta SP:

      +--------------------+    
      |  valor             |                                       
      +--------------------+ 
      | Parámetro N        |   
      +--------------------+     
      | Parámetro N-1      |     
      +--------------------+
      | .....              |  
      +--------------------+
      | Parámetro 2        |  
      +--------------------+
      | Parámetro 1        |  
      +--------------------+
      |  IP                | 
      | (Dir. de retorno)  |   
      +--------------------+
      | Valor de  BP       | Se almacena el valor de BP en la pila
      +--------------------+ <-- SP  <-- BP                       
      |                    |                              
      |  Espacio           |    
      |  disponible en     |
      |  la pila           |
      |                    |
      |                    |
      +--------------------+ <---  SS apunta a la base de la pila 
      |                    |
      |                    |
      +--------------------+


Con esta instrucción se termina el proceso de crear el marco de pila. Ahora es totalmente seguro decrementar el valor de SP con el propósito de crear espacios para las variables locales a la rutina, o de insertar otros valores en la pila.

Por ejemplo, la instrucción 

    subw $4, %sp /* Sintaxis AT&T */  

ó 

    sub sp, 4  /* Sintaxis Intel */


Crea un espacio de 4 bytes (2 words) en la pila, que ahora se encontrará así:

      +--------------------+    
      |  valor             |                                       
      +--------------------+ 
      | Parámetro N        |   
      +--------------------+     
      | Parámetro N-1      |     
      +--------------------+
      | .....              |  
      +--------------------+
      | Parámetro 2        |  
      +--------------------+
      | Parámetro 1        |  
      +--------------------+
      |  IP                | 
      | (Dir. de retorno)  |   
      +--------------------+
      | Valor de  BP       |                                        
      +--------------------+ <-- BP            
      | Espacio disponible |  
      +--------------------+
      | Espacio disponible |  
      +--------------------+ <-- SP            
      |                    |                              
      |  Espacio           |    
      |  disponible en     |
      |  la pila           |
      |                    |
      |                    |
      +--------------------+ <---  SS apunta a la base de la pila 
      |                    |
      |                    |
      +--------------------+


Observe que ESP se decrementa, pero BP sigue apuntando al inicio del marco de pila. Por esta razón, el puntero BP se denomina Base Pointer (puntero base), ya que con respecto a él es posible acceder tanto los parámetros enviados a la rutina, como las variables locales creadas en ésta.

Por ejemplo, la instrucción 

    movw 4(%bp), %ax /* Sintaxis AT&T */

ó
 
    mov ax, WORD PTR [ bp + 4 ] /* Sintaxis Intel */


Mueve el contenido de la memoria en la posición SS:[BP + 4] al registro AX, es decir que almacena el primer parámetro pasado a la rutina en el registro AX.

A continuación se presenta de nuevo el estado actual de la pila, para visualizar los diferentes desplazamientos a partir del registro BP.

      +--------------------+    
      |  valor             |                                       
      +--------------------+ 
      | Parámetro N        |   
      +--------------------+ <-- BP + 2*N      
      | Parámetro N-1      |     
      +--------------------+ ...
      | .....              |  
      +--------------------+ <-- BP + 8
      | Parámetro 2        |  
      +--------------------+ <-- BP + 6
      | Parámetro 1        |  
      +--------------------+ <-- BP + 4
      |  IP                | 
      | (Dir. de retorno)  |   
      +--------------------+ <-- BP + 2
      | Valor de  BP       |                                        
      +--------------------+ <-- BP            
      | Espacio disponible |  
      +--------------------+ <-- BP - 1
      | Espacio disponible |  
      +--------------------+ <-- SP  <-- BP - 4        
      |                    |                              
      |  Espacio           |    
      |  disponible en     |
      |  la pila           |
      |                    |
      |                    |
      +--------------------+ <---  SS apunta a la base de la pila 
      |                    |
      |                    |
      +--------------------+


De esta forma, la instrucción 

    movw %ax, -2(%bp) /* Sintaxis AT&T */

ó
 
    mov WORD PTR [ bp - 2 ], ax /* Sintaxis Intel */


Almacena el valor del registro AX en el primer word de espacio de la pila.

Se puede observar que si se crea un marco de pila estándar con las instrucciones mencionadas, siempre el primer parámetro que se paso a la rutina se encontrará en (BP + 4), el segundo en (BP + 6) y así sucesivamente.

Se debe tener en cuenta que si dentro de la rutina se modifica el valor del registro BP, se deberá almacenar su valor con anterioridad en la pila o en otro registro.

Cerrar el marco de Pila
------------------------


Al finalizar la rutina se deberá cerrar el marco de pila creado. La instrucción
 
     movw %bp, %sp /* Sintaxis AT&T */

ó 
    mov sp, bp  /* Sintaxis Intel */


Cierra el espacio creado para las variables locales, al apuntar SP a la misma dirección de memoria en la pila a la que BP. Luego de esta instrucción la pila lucirá así:

      +--------------------+    
      |  valor             |                                       
      +--------------------+ 
      | Parámetro N        |   
      +--------------------+     
      | Parámetro N-1      |     
      +--------------------+
      | .....              |  
      +--------------------+
      | Parámetro 2        |  
      +--------------------+
      | Parámetro 1        |  
      +--------------------+
      |  IP                | 
      | (Dir. de retorno)  |   
      +--------------------+
      | Valor de  BP       |                                        
      +--------------------+ <-- BP  <-- SP  
      |                    |  
      +--------------------+
      |                    |  
      +--------------------+ <-- SP antes de cerrar el marco de pila
      |                    |                              
      |  Espacio           |    
      |  disponible en     |
      |  la pila           |
      |                    |
      |                    |
      +--------------------+ <---  SS apunta a la base de la pila 
      |                    |
      |                    |
      +--------------------+


En este momento ya no es seguro acceder a los valores almacenados en el espacio para variables locales.

Ahora se deberá recuperar el valor original de BP antes de crear el marco de pila: 

    popw %bp /* Sintaxis AT&T */

ó 
    pop bp /* Sintaxis Intel */


Con ello la pila se encontrará en el siguiente estado:

      +--------------------+    
      |  valor             |                                       
      +--------------------+ 
      | Parámetro N        |   
      +--------------------+     
      | Parámetro N-1      |     
      +--------------------+
      | .....              |  
      +--------------------+
      | Parámetro 2        |  
      +--------------------+
      | Parámetro 1        |  
      +--------------------+
      |  IP                | 
      | (Dir. de retorno)  |   
      +--------------------+ <-- SP
      | Valor de  BP       | --> Este valor sale de la pila y se almacena en BP
      +--------------------+     
      |                    |                              
      |  Espacio           |    
      |  disponible en     |
      |  la pila           |
      |                    |
      |                    |
      +--------------------+ <---  SS apunta a la base de la pila 
      |                    |
      |                    |
      +--------------------+


Ahora SP apunta a la dirección de retorno de la rutina (donde debe continuar la ejecución). La instrucción
 
    ret


Toma de la pila la dirección de retorno (la dirección de memoria de la instrucción siguiente a la cual se llamó la rutina mediante call), y realiza un jmp a esa dirección.

Note que luego de retornar de la rutina, la pila se encontrará en el siguiente estado:

      +--------------------+    
      |  valor             |                                       
      +--------------------+ 
      | Parámetro N        |   
      +--------------------+     
      | Parámetro N-1      |     
      +--------------------+
      | .....              |  
      +--------------------+
      | Parámetro 2        |  
      +--------------------+
      | Parámetro 1        |  
      +--------------------+ <-- SP  (los parámetros continúan en la pila)
      | IP                 | --> Este valor sale de la pila y se almacena en IP
      +--------------------+                             
      |  Espacio           |    
      |  disponible en     |
      |  la pila           |
      |                    |
      |                    |
      +--------------------+ <---  SS apunta a la base de la pila 
      |                    |
      |                    |
      +--------------------+


Por esta razón es necesario avanzar SS en un valor igual al número de bytes que se enviaron como parámetro a la rutina. Si se enviaron N parámetros a la pila, el numero de bytes que se deberán sumar a sp son 2 * N (En modo real cada parámetro ocupa un word = 2 bytes).
 
    addw $M, %sp /* Sintaxis AT&T */
    
ó 

    add sp, M /* Sintaxis Intel */

Donde M representa el número de bytes a desplazar SP.

Con estas instrucciones la pila se encontrará en el mismo estado que antes de invocar la rutina:

      +--------------------+    
      |  valor             |                                       
      +--------------------+ <- ESP (tope de la pila)
      |                    |
      |                    |
      |                    |
      |                    |
      |                    |    
      |                    |     
      |                    |                              
      |  Espacio           |
      |  disponible en     |
      |  la pila           |
      |                    |
      |                    |
      +--------------------+ <---  SS apunta a la Base de la Pila
      |                    |
      |                    |
      +--------------------+

Plantilla de Rutina en Modo Protegido de 32 bits
-------------------------------------------------

La plantilla de una rutina en modo protegido de 32 bits es muy similar a la de modo real. La principal diferencia entre las dos consiste en el tamaño de los registros, que se expanden de 16 a 32 bits (BP se expande a EBP y SP se expande a ESP).

También es importante recordar que cada parámetro almacenado en la pila ocupa 4 bytes (32 bits), por lo cual el valor que se debe sumar a ESP después de retornar de la rutina es 4 * el número de parámetros insertados.

En sintaxis AT&T

    nombre_rutina:
       pushl %ebp   /*Almacenar %ebp en la pila*/
       movw %esp, %ebp  /*Establecer %ebp con el valor de %esp*/
        /*Ya se ha creado un marco de pila*/
       ...
       (instrucciones de la rutina)
       Por ejemplo, para obtener el primer parámetro de la pila sería:
       mov 8(%ebp), %eax
       ...
    
        /*Cerrar el marco de pila:*/
       movw %ebp, %esp  /*Mover %ebp a %esp*/
       popw %ebp/*Recuperar el valor original de %ebp */
    
       ret /* Retornar de la rutina */

En sintaxis Intel

    nombre_rutina:
       push ebp   /*Almacenar ebp en la pila*/
       mov ebp, esp  /*Establecer ebp con el valor de esp*/
        /*Ya se ha creado un marco de pila*/
       ...
       (instrucciones de la rutina)
       Por ejemplo, para obtener el primer parámetro de la pila sería:
       mov eax, [ebp + 8]
       ...
    
        /*Cerrar el marco de pila:*/
       mov esp, ebp  /*Mover ebp a esp*/
       pop ebp/*Recuperar el valor original de ebp */
    
       ret  /* Retornar de la rutina*/
