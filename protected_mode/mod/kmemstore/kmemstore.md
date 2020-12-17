# Almacenes de memoria (memstore)

Este módulo contiene las funciones para crear y gestionar almacenes de
memoria de tamaño arbitrario. Un almacén de memoria usa uno o varios
almacenes de bloques (pools) del tamaño especificado cuando se crea el
almacén. A medida que el almacén de bloques interno se llena, se van
adicionando nuevos almacenes de bloques de forma automática.

## Dependencias
- kpool
- paging

## Subrutina de inicialización
- Ninguna.

