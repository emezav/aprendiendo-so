Multitarea
==========

Nota de alineacion con la serie `protected_mode`
------------------------------------------------

Este documento describe la arquitectura general hacia la cual apunta la serie
de `protected_mode`, no una fotografia exacta de todas las etapas desde
`04_irq_console`.

Lectura recomendada segun el roadmap actual:

- `06_kernel_threads`: introduce hilos de kernel y cambio de contexto
  cooperativo
- `07_scheduler`: agrega planificacion por timer
- `08_user_mode`: agrega transicion estable a `ring 3`
- `09_syscalls`: agrega ABI minima sobre `int 0x80`
- `10_elf_loader`: reemplaza la demo embebida por carga real desde `ELF32`
- `11_ramfs` y `12_user_shell`: deben completar la fuente de archivos y la
  experiencia tipo mini Unix

Por eso, algunas secciones de este documento ya describen mecanismos que hoy
existen en la serie, mientras que otras describen piezas todavia proyectadas,
como una capa mas completa de procesos, filesystem y shell.

Entorno minimo para ejecutar tareas
-----------------------------------

El Sistema Operativo debe inicializar un entorno en el cual las tareas puedan
ejecutarse. Conceptualmente, este entorno esta constituido por:

1. Un Segmento de Estado de Tarea (TSS) accesible desde la GDT
2. Selectores y descriptores para codigo y datos de kernel y usuario
3. Un mecanismo para crear una tarea a partir de un archivo ejecutable
4. Un mecanismo para pasar el control de la CPU a las tareas
5. Un mecanismo para retornar el control de la CPU al sistema operativo
6. Un mecanismo de comunicacion entre las tareas y el sistema operativo

En la serie actual, esos puntos aparecen de forma progresiva:

- la TSS minima para transiciones desde usuario ya existe desde
  `08_user_mode`
- los selectores de usuario y kernel ya existen y son funcionales en `10`
- la carga desde ejecutables ELF ya existe en `10_elf_loader`
- la capa mas completa de procesos y filesystem aun pertenece a etapas
  posteriores del roadmap

Estructura de datos de una tarea
--------------------------------

El sistema operativo suele almacenar la informacion de cada tarea en una
estructura de datos que permita identificarla y gestionarla. Algunos campos
clasicos son:

- PID
- PPID
- estado de la tarea
- nivel de privilegios
- punto de entrada
- base y limite logicos de su memoria
- referencias al estado del procesador

A esta informacion suele llamarsela Bloque de Control de Proceso, BCP o PCB.

En `06` a `10`, la serie todavia no implementa un BCP/PCB completo con todos
estos campos. Lo que si existe hoy es:

- infraestructura de hilos de kernel
- scheduler minimo observable
- una transicion real a `ring 3`
- una ABI minima de syscalls
- carga de un unico programa ELF de usuario para demostracion

Campos como `PPID`, estados de bloqueo mas ricos, tabla de procesos completa o
heap por proceso deben leerse aqui como arquitectura proyectada hacia etapas
posteriores.

Segmento de Estado de Tarea
---------------------------

Se debe crear un Segmento de Tarea (Task Segment). Esta estructura permite al
procesador conocer informacion de contexto privilegiado, especialmente durante
transiciones entre niveles de privilegio.

En la implementacion real de la serie actual no se usa un TSS distinto por
tarea. Desde `08_user_mode` en adelante se usa una TSS minima del kernel para
resolver la transicion desde `ring 3` hacia `ring 0`, especialmente para
proveer `esp0` y `ss0` al procesador durante la entrada al kernel.

Selectores y segmentacion
-------------------------

El esquema clasico de x86 describe:

- selector de codigo de kernel
- selector de datos de kernel
- selector de TSS
- selectores de codigo y datos de usuario

Esto sigue siendo valido como marco conceptual. Sin embargo, la decision
vigente de la serie actual no usa segmentos con `base` y `limit` distintos por
tarea para aislar memoria.

En `08`-`10`, la separacion real entre kernel y usuario descansa en:

- segmentacion plana para kernel y usuario
- descriptores de usuario con `base = 0` y limite amplio
- aislamiento real por paginacion y flags de usuario/supervisor

Por eso, el modelo de "segmentos por tarea" debe entenderse aqui como una
descripcion historica o teorica util, no como descripcion literal del codigo
actual.

Creacion de una tarea desde un archivo ejecutable
-------------------------------------------------

Para crear una tarea a partir de un archivo ejecutable, se debe considerar:

- el formato del ejecutable
- la fuente desde la cual se obtiene
- el layout virtual en memoria
- el mecanismo de transferencia de control

En esta serie se adopto `ELF32` como formato inicial de programas de usuario.

Estado respecto al roadmap:

- `10_elf_loader` ya valida y carga `ELF32`
- `10_elf_loader` ya usa un modulo multiboot como fuente externa preferida
- `11_ramfs` debe introducir una fuente de archivos en memoria para reemplazar
  gradualmente esa dependencia directa de modulos

Carga de modulos de GRUB
------------------------

Una forma sencilla de entregar ejecutables al kernel consiste en usar modulos
multiboot cargados por GRUB junto con el kernel. Conceptualmente, el flujo es:

1. GRUB carga el kernel
2. GRUB carga uno o mas modulos adicionales
3. la estructura multiboot expone `mods_count` y `mods_addr`
4. el kernel inspecciona esos modulos y decide cual usar

Este mecanismo ya esta alineado con `10_elf_loader`, donde el primer modulo ELF
valido se toma como fuente externa preferida del programa de usuario.

Archivos ejecutables en formato ELF
-----------------------------------

Un archivo ejecutable en formato ELF contiene un encabezado principal y una
tabla de `program headers` que permite identificar los segmentos necesarios
para la ejecucion.

Para la serie actual, el subconjunto soportado se mantiene deliberadamente
pequeno:

- `ELF32`
- little-endian
- `EM_386`
- `ET_EXEC`
- segmentos `PT_LOAD`
- sin relocaciones
- sin enlazado dinamico

A partir de esa informacion, el kernel puede:

- validar la imagen
- ubicar el `entry point`
- mapear paginas de usuario
- copiar `p_filesz`
- limpiar en cero hasta `p_memsz`
- preparar la pila de usuario

En `10_elf_loader`, la parte ya implementada es precisamente esa.

Ubicacion de una tarea en memoria
---------------------------------

Desde el punto de vista conceptual, una tarea puede verse como una combinacion
de:

- codigo
- datos
- heap
- pila

Si no se usa memoria virtual, la region de la tarea suele pensarse como un
bloque continuo. Si se usa memoria virtual con paginacion, la region no tiene
que ser contigua en memoria fisica, aunque si debe ser coherente en su espacio
lineal.

Esta segunda interpretacion es la mas cercana a la arquitectura actual de la
serie. En `10`, el programa de usuario se construye a partir de paginas
virtuales de usuario y marcos fisicos separados, con aislamiento apoyado en la
MMU y no en segmentos con base y limite por tarea.

Todavia no existe una abstraccion completa de heap por proceso ni una tabla de
procesos general.

Transferencia de control hacia una tarea
----------------------------------------

Una vez cargada y organizada la tarea en memoria, el sistema debe establecer un
entorno inicial de ejecucion que sea compatible con la rutina de retorno desde
interrupcion o con el `iret` usado para entrar a `ring 3`.

La idea general es preparar un marco de pila consistente con:

- selectores correctos
- `EIP`
- `EFLAGS`
- pila de usuario
- registros generales

Esta parte si esta alineada con la serie actual. Desde `08_user_mode` y luego
en `09` y `10`, el salto a `ring 3` y el retorno controlado al kernel dependen
precisamente de construir y restaurar correctamente ese entorno de ejecucion.

Observacion importante para implementaciones progresivas
--------------------------------------------------------

Si se reaprovecha una rutina generica de retorno desde interrupcion, el marco
de pila inicial de una tarea o hilo nuevo debe coincidir exactamente con el
formato que dicha rutina restaura.

En particular, si la rutina de retorno realiza operaciones equivalentes a:

1. restaurar `gs`, `fs`, `es`, `ds`
2. restaurar registros generales con una secuencia tipo `popa`
3. descartar `number` y `error_code`
4. ejecutar `iret`

entonces el marco debe estar alineado exactamente a ese contrato.

Si el contexto inicial contiene palabras extra, campos omitidos o mezcla el
formato de una tarea de kernel con el de una transicion a otro nivel de
privilegios, el resultado tipico sera:

- `General Protection Fault`
- `Page Fault`
- retorno a una direccion invalida

Esta observacion se vuelve especialmente importante al separar dos casos:

- hilos de kernel que retornan a `ring 0`
- tareas de usuario que retornan a `ring 3`, donde si se requieren
  `old_esp` y `old_ss`

Por esa razon conviene documentar y validar por separado el formato exacto del
marco para cada etapa de la serie.

Retorno del control al sistema operativo
----------------------------------------

Cuando una tarea recibe el control de la CPU, existen al menos dos estrategias
pedagogicamente utiles para retornar al kernel:

- invocar una interrupcion por software
- dejar que una interrupcion de timer fuerce la entrada al kernel

Esto permite que dentro de las rutinas de interrupcion el sistema decida si:

- retorna a la misma tarea
- conmuta a otra
- termina la tarea
- vuelve al monitor o al scheduler

En el estado actual de la serie, esta logica existe de forma parcial y
pedagogica:

- el timer ya puede disparar alternancia entre hilos de kernel
- `SYS_YIELD` ya permite cesion cooperativa desde usuario
- `SYS_EXIT` ya permite retornar desde el programa de usuario al monitor

Una politica mas completa de procesos, bloqueo por eventos y espera de hijos
queda para etapas posteriores.

Llamadas al sistema
-------------------

El mecanismo basico de comunicacion entre las tareas y el sistema operativo es
la interrupcion. Las tareas cargan parametros en registros o memoria de usuario
y luego invocan una entrada privilegiada que entrega el control al kernel.

Este mecanismo es conceptualmente similar al usado por sistemas reales. En la
serie actual, la ABI minima ya visible desde `09_syscalls` y reutilizada por
`10_elf_loader` usa `int 0x80` con:

- `eax` como numero de syscall
- `ebx` y `ecx` como primeros parametros
- retorno en `eax`

Sobre esa base ya existen de forma observable:

- `SYS_WRITE`
- `SYS_YIELD`
- `SYS_EXIT`
- `SYS_GETPID`

Resumen pedagogico
------------------

Si este documento se lee junto con el roadmap, la interpretacion correcta es:

- la teoria general de multitarea aparece aqui de forma unificada
- la implementacion real se distribuye por etapas pequenas en `06` a `12`
- `10_elf_loader` ya cubre hilos, scheduler minimo, transicion a usuario,
  syscalls y carga real de `ELF32`
- filesystem, shell y una capa mas completa de procesos siguen siendo trabajo
  de las etapas posteriores
