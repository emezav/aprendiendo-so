# Aprendiendo Sistemas Operativos para procesadores Intel

Aprendiendo SO - Aprendizaje de las bases iniciales para el estudio y/o
construcción de Sistemas Operativos, usando como caso de estudio los
procesadores de arquitectura Intel (x86 y x86-64).

# Estructura del software

La serie Aprendiendo SO está conformada por un conjunto de proyectos
de programación que pueden ser compilados y ejecutados de forma independiente. 
Para facilitar su estudio, los proyectos se encuentran construidos de forma
acumulativa, adicionando alguna característica o funcionalidad (por ejemplo la
gestión de memoria virtual, implementación de multi tarea, llamadas al sistema,
lectura/escritura de disco, entrada por teclado, etc.).

A partir de los proyectos base se puede modificar el código existente para
implementar otras funcionalidades o características que permitan estudiar y
comprender los sistemas Intel y los fundamentos de los sistemas operativos.

# Requerimientos

Para una descripción general de los requerimientos para ejecutar los proyectos,
vea [Entorno de desarrollo y ejecución] (dox/development_environment.md).

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

