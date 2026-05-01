Observabilidad del kernel
=========================

En los proyectos de `protected_mode` conviene distinguir tres tipos de salida:

1. salida para la persona que interactua con el kernel en VGA
2. salida para depuracion y analisis posterior por puerto serial
3. salida que debe verse en ambos canales

Convencion recomendada
----------------------

La convencion sugerida para la serie es:

- `console_printf(...)`
  - usar cuando el mensaje pertenece principalmente a la interfaz visible del
    kernel en pantalla
- `serial_printf(...)`
  - usar cuando el mensaje es principalmente una traza tecnica o de
    instrumentacion
- `klog_printf(...)`
  - usar cuando el mismo mensaje debe llegar tanto al usuario humano en VGA
    como al analisis posterior por `serial.log`

Motivacion
----------

Esta separacion ayuda a mantener una intencion clara:

- no todo mensaje de consola debe contaminar el log serial
- no toda traza serial debe saturar la pantalla
- algunos eventos importantes deben quedar disponibles en ambos lugares

Ejemplos tipicos para `klog_printf`
----------------------------------

- resultados de comandos del monitor
- mensajes de pruebas pedagogicas
- confirmaciones visibles de que una demo termino correctamente

Ejemplos tipicos para `serial_printf`
------------------------------------

- trazas de teclado por scancode
- detalles de cambio de contexto
- mensajes de instrumentacion de bajo nivel
- eventos demasiado verbosos para la VGA

Ejemplos tipicos para `console_printf`
-------------------------------------

- prompts del monitor
- ayuda interactiva
- mensajes puramente de interfaz local

Nota sobre pruebas manuales
---------------------------

Durante pruebas en QEMU, el archivo `build/serial.log` se vuelve una fuente
muy valiosa para validar el comportamiento del kernel, incluso cuando la
interaccion principal ocurre en la pantalla VGA.

Por esa razon, cuando una accion de prueba sea importante tanto para la persona
como para el analisis posterior, conviene preferir `klog_printf(...)`.
