# Aprendiendo Sistemas Operativos para procesadores Intel			{#mainpage}

Aprendiendo SO - Aprendizaje de las bases iniciales para el estudio y/o
construcción de Sistemas Operativos, usando como caso de estudio los
procesadores de arquitectura Intel (x86 y x86-64).

# Estructura del software

La serie Aprendiendo SO está conformada por un conjunto de proyectos
de programación que pueden ser compilados y ejecutados de forma independiente. 
Para facilitar el aprendizaje, los proyectos se encuentran construidos de forma
acumulativa, adicionando alguna característica o funcionalidad (por ejemplo la
gestión de memoria virtual, implementación de multi tarea, llamadas al sistema,
lectura/escritura de disco, entrada por teclado, etc.).

A partir de los proyectos base, el profesor o estudiante puede modificar el
código existente para implementar otras funcionalidades o características que
permitan estudiar y comprender los sistemas Intel y los fundamentos de los
sistemas operativos.

# Requerimientos

Para compilar y ejecutar los proyectos se requiere:
- Editor de texto / Entorno de desarrollo para visualizar / modificar el código
- Compilador / Ensamblador / Linker : Es necesario contar con un compilador que
  permita obtener un ejecutable en formato ELF de 32 bits. Estos requisitos son
  cumplidos fácilmente en un sistema GNU/Linux de 32 bits.
  + Compilador Gcc
  + Ensamblador As
  + Linker Ld
  En otras plataformas se debe usar un compilador cruzado.
- Emulador: Es necesario contar con los emuladores qemu y bochs. Este último
  debe tener habilitada la depuración gráfica.
- Otras herramientas: La compilación de los proyectos se basa en Makefiles, por
  lo cual es necesario contar con la utilidad Make.
- E2fsimage: Esta utilidad permite crear una imagen de disco. Se encuentra
  disponible en la mayoría de distribuciones de Linux, o puede ser instalada
  desde código fuente.
- dd (Rawrite en Windows): Esta utilidad se usa para manipular la imagen de
  disco generada.

# Compilación y ejecución de los proyectos

Cada proyecto cuenta con un Makefile. Los _targets_ que se usan para compilar o
ejecutar el proyecto son:
- __all__: (Regla por defecto). Compila el código fuente y crea la imagen de
  disco.
- __clean__: Elimina los archivos resultado de la compilación y la documentación
  generada.
- __docs__: Genera la documentación del proyecto.
- __qemu__: Compila el proyecto y crea la imagen de disco si es necesario, y
  luego inicia el emulador qemu con la imagen de disco generada.
_ __bochs__: Similar a la regla __qemu__, pero inicia el emulador _bochs_.
- __bochsdbg__: Igual a la anterior, pero invoca a _bochs_ iniciando el
  depurador gráfico. _Para usar esta regla, se debe tener el depurador gráfico
  habilitado en bochs_.


# Aviso de responsabilidad / disclaimer

La serie Aprendiendo Sistemas Operativos ha sido y está siendo construida a
partir de los documentos proporcionados por Intel, la documentación recopilada
de otras fuentes (libros, internet, etc.), y la experiencia adquirida en la
enseñanza de la asignatura Sistemas Operativos en un entorno universitario.
Representa el trabajo personal del autor, quien hace su mejor esfuerzo por
actualizar y mejorar los proyectos, y mantenerlos libres de errores o
inconsistencias. 

Aprendiendo SO se ofrece sin ningún tipo de garantía implícita o explícita, por
lo cual el autor no se hace responsable por cualquier daño o perjuicio causado
por el uso parcial o total del material aquí dispuesto. 
