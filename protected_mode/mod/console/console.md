# Interfaz de salida por consola

Este módulo adiciona la funcionalidad para imprimir texto en la memoria de
video de modo texto.

## Dependencias
Ninguna.

## Subrutina de inicialización
- setup_console


# Memoria de video en Modo Texto

La memoria de video en modo texto a color (25 líneas de 80 caracteres cada una)
se encuentra mapeada en memoria a la dirección física 0xB8000, y ocupa 32 KB de 
memoria. Cada carácter en que se muestra en la pantalla ocupa 2 bytes en la 
memoria de video (un word): un byte contiene el código ASCII del carácter, y 
el otro byte contiene los atributos de color de texto y color de fondo del 
carácter.  A su vez este byte se subdivide en:

        7  6  5  4  3  2  1  0    
        +-----------------------+
        |I |B |B |B |I |F |F |F |
        +-----------------------+
        Los bits F correspondel al color del texto (Foreground).
        Los bits B corresponden al color de fondo (Background).
        El bit I corresponde a la intensidad del color de fondo (0 = oscuro,
        1 = claro) o del color del texto.

De esta forma, para mostrar un carácter en la esquina superior de la pantalla 
(línea 0, carácter 0) se deberá escribir un word (2 bytes) en la dirección
física 0xB8000. El primer byte de este word será el código ascii a mostrar, y 
los siguientes bytes representarán el color de texto y de fondo del carácter.  

El siguiente word (ubicado en la dirección de memoria 0xB8002) corresponde al 
segundo carácter en la pantalla, y así sucesivamente.

Los colores válidos se muestran en la siguiente tabla :

| Valor | Color |  Valor |  Color |
| ---- | ---- |  ---- |  ---- |
| 0 |     black |  8     |  dark gray |
| 1 |     blue  |  9     |  bright blue |
| 2 |     green |  10    |  bright green |
| 3 |     cyan  |  11    |  bright cyan |
| 4 |     red   |  12    |  pink |
| 5 |     magenta| 13    |  bright magenta |
| 6 |     brown  | 14    |  yellow |
| 7 |    white   | 15    |  bright white |

Los colores 0-15 son válidos para el color de texto. Sin embargo para el fondo 
solo es posible utilizar los colores del 0 al 7,. Los colores de fondo  8 al 15
en algunas tarjetas causan el efecto 'blink' (parpadeo) del texto. 

Por ejemplo, para imprimir el carácter 'A', (al cual le corresponde el código 
ASCII 65, 0x41 en hexadecimal) con color blanco sobre fondo negro en la esquina
superior de la pantalla, se deberá copiar el word 0x0F41 en la dirección de 
memoria 0xB8000. El byte de atributos 0x0F indica texto blanco (dígito hexa F) 
sobre fondo negro (dígito hexa 0).

Se puede observar la posición XY de un carácter en la pantalla se puede obtener
de la siguiente forma:

$Pos_XY = 0xB8000 + ((80*Y) + X) *2$

En donde 0xB8000 es la dirección física de la memoria de video (esquina superior
izquierda), Y representa la fila, X representa la columna. Se debe multiplicar
por 2 debido a que cada carácter en la pantalla en realidad ocupa 2 bytes,
uno para el código ascii y otro para los atributos de color de texto y fondo.

## Actualización de la Posición del Cursor

El controlador CRT (controlador de video) cuenta con los siguientes puertos de 
Entrada y Salida:

| Dirección |  Descripción |
|-----------|--------------|
|  0x3D4   | registro de índice |
|  0x3D5   | registro de control |

Para programar el microcontrolador CRT, primero se debe escribir en el 
registro de índice. El valor escrito en este puerto le indica al controlador
el parámetro que se desea configurar. Luego se debe escribir en el registro
de control el dato requerido.

En el caso de la configuración de la posición del cursor, se debe escribir en
los puertos del CRT la posición lineal Pos_XY. Debido a que esta posición 
requiere 16 bits y a que los puertos de E/S del CRT son de 8 bits (1 byte), 
se deben escribir primero los 8 bits menos significativos de la posición lineal
y luego los 8 bits más significativos. Este proceso se describe a continuaión:

- Escribir el valor 0x0F (Cursor Location Low) en el registro de índice del 
   CRT (puerto de E/S 0x3D4).
- Escribir los 8 bits menos significativos de la posición lineal del cursor
   al registro de control del CRT (puerto 0x3D5).
- Escribir el valor 0x0E (Cursor Location High) en el registro de índice del 
   CRT (puerto de E/S 0x3D4).
- Escribir los 8 bits más significativos de la posición lineal del cursor
   al registro de control del CRT (puerto 0x3D5). 

## Vea también 
- http://www.osdever.net/FreeVGA/vga/crtcreg.htm Puertos (registros) del
  controlador CRT

