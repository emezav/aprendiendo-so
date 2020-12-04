# Gestión de la memoria física.

Este módulo contiene las funciones para gestionar la memoria física del
sistema.

## Dependencias
- bitmap

## Subrutina de inicialización
- setup_physical_memory: Esta subrutina debe ser invocada antes de
	configurar y habilitar las interrupciones (setup_interrupts).

# Generalidades de la gestión de la memoria física

La gestión de memoria es el mecanismo de asignar y liberar unidades de memoria 
de forma dinámica.  Para ofrecer esta funcionalidad, es necesario contar con 
una estructura de datos que permita llevar un registro de la memoria que se
encuentra asignada y la memoria libre.

Los mapas de bits son un mecanismo para gestionar memoria que se basan en un 
principio simple: Usando un bit (cuyo valor puede ser cero o uno) se puede
determinar si un byte o una región de memoria se encuentra disponible o no.
Esto ofrece una posibilidad sencilla para gestionar memoria, pero se puede
ver limitada por el tamaño del mapa de bits en sí.

Por ejemplo, si se desea gestionar una memoria de 4 GB (2^32 bytes) y se usa
un bit por cada byte de memoria (tomando la unidad básica de asignación como
un byte), el mapa de bits ocuparía 2^29 bits, es decir 512 MB.

Para evitar que el mapa de bits tenga un tamaño considerable con respecto a
la cantidad de memoria a administrar, con frecuencia se usa unidades
de asignación mayores a un byte. Por ejemplo, si se crea un mapa de bits
en el cual cada uno de ellos representa una región de memoria (unidad de
asignación) de 4 KB (2^12 bytes), el mapa de bits correspondiente para una
memoria de 4 GB ocuparía exactamente 128 KB. Este tamaño es aceptable, pero
causa que no se puedan asignar unidades de memoria menores a 4 KB.

A continuación se presenta una descripción gráfica del uso de un mapa de bits.

               Esquema del Mapa de Bits
 
    +-----------------------------------+        Cada bit en el mapa de bits
    | 1| 0| 1| 0| 1| 0|..|..|..|..| 0| 1|        representa una unidad de
    +-----------------------------------+        asignación de memoria  
     
    +-------------------------------------------------------------------------+
    |libre |usada|libre|usada|libre|usada|...  |     |     |     |usada|libre |
    |      |     |     |     |     |     |     |     |     |     |     |      |
    +-------------------------------------------------------------------------+

## Creación del mapa de bits

El mapa de bits inicialmente se llena de unos, para indicar todo el espacio
de memoria como disponible. Luego a partir de la información de la memoria
disponible se "toman" (usan) las unidades y los bits correspondientes se
marcan con cero.
 
## Asignación de memoria

La asignación de memoria se puede realizar de dos formas:

- Asignar una unidad de memoria: Se recorre el mapa de bits buscando 
  un bit que se encuentre en 1 (región disponible). Si se encuentra este bit,
  se obtiene el inicio de la dirección de memoria que éste representa y se 
  retorna.

                  Asignar una unidad de memoria
    
	  +-------- Esta entrada (bit) en el mapa de bits se encuentra en 1.
      |         Esto significa que la región asociada a este bit está
      v         disponible.  
     +-----------------------------------+        Cada bit en el mapa de bits
     | 1| 0| 1| 0| 1| 0|..|..|..|..| 0| 1|        representa una unidad de
     +-----------------------------------+        asignación de memoria  
     
    +-------------------------------------------------------------------------+
    |libre |usada|libre|usada|libre|usada|...  |     |     |     |usada|libre |
    |      |     |     |     |     |     |     |     |     |     |     |      |
    +-------------------------------------------------------------------------+
      ^
      |
      +---------- Región de memoria representada por el primer bit. Se debe 
                  retornar la dirección de memoria de inicio de la región.


      +------------- La entrada se marca como "no disponible"             
      |              
      v  
    +-----------------------------------+      Mapa de bits actualizado  
    | 0| 0| 1| 0| 1| 0|..|..|..|..| 0| 1|        
    +-----------------------------------+          
  
- Asignar una región de memoria de N bytes: Primero se redondea el tamaño 
  solicitado a un múltiplo del tamaño de una unidad de asignación. Luego se 
  busca dentro del mapa de bits un número consecutivo de bits que sumen la
  cantidad de memoria solicitada. Si se encuentra, se marcan todos los bits
  como no disponibles y Se retorna la dirección física que le corresponde al 
  primer bit en el mapa de bits. 

                  Asignar una región de memoria
                  
            +-------------  Este es el inicio de la región de memoria
            |               disponible
            v                
     +-----------------------------------+        Cada bit en el mapa de bits
     | 1| 0| 1| 1| 1| 1|..|..|..|..| 0| 1|        representa una unidad de
     +-----------------------------------+        asignación de memoria  
     
     +-------------------------------------------------------------------------+
     |libre |usada|libre|libre|libre|libre|...  |     |     |     |usada|libre |
     |      |     |     |     |     |     |     |     |     |     |     |      |
     +-------------------------------------------------------------------------+
                    ^
                    |
                    +---------- Inicio de la región de memoria. Se retorna la 
                                dirección que le corresponde al primer bit del
								mapa.

             +-------------  La región de memoria se marca como no disponible
             |               
             v                
     +-----------------------------------+        Se deben marcar los bits
     | 1| 0| 0| 0| 0| 0|..|..|..|..| 0| 1|        correspondientes como   
     +-----------------------------------+        "no disponible"   

## Liberación de memoria

Para liberar memoria se puede simplemente establecer en 1 (disponible) el bit
correspondiente a la unidad o la región a liberar. 

                  Liberar una región de memoria
                  
          +-------------  Este es el inicio de la región de memoria
          |               asignada
          v                
     +-----------------------------------+        Cada bit en el mapa de bits
     | 1| 0| 0| 0| 0| 0|..|..|..|..| 0| 1|        representa una unidad de
     +-----------------------------------+        asignación de memoria  
       
    +-------------------------------------------------------------------------+
    |libre |usada|usada|usada|usada|usada|...  |     |     |     |usada|libre |
    |      |     |     |     |     |     |     |     |     |     |     |      |
    +-------------------------------------------------------------------------+
                  ^
                  |
                  +---------- Inicio de la región de memoria a liberar
    
  
             +-------------  La región de memoria se marca como  disponible
             |               
             v                
      +-----------------------------------+        Se deben marcar los bits
      | 1| 0| 1| 1| 1| 1|..|..|..|..| 0| 1|        correspondientes como   
      +-----------------------------------+        "no disponible"   

