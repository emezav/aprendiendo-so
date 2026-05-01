/**
 * @file
 * @ingroup kernel_code
 * @brief API minima de lectura de archivos sobre ext2 en solo lectura.
 */

#ifndef EXT2_H_
#define EXT2_H_

#define EXT2_PATH_MAX 255
#define EXT2_FILE_READ_MAX 16384
#define EXT2_LIST_MAX 4096

/** @brief Inicializa el estado minimo del lector ext2. */
void setup_ext2(void);

/** @brief Registra los comandos de monitor asociados a ext2. */
void register_ext2_monitor_commands(void);

/** @brief Indica si ext2 puede usarse sobre el disco seleccionado. */
int ext2_is_ready(void);

/**
 * @brief Resuelve un path absoluto de ext2 y retorna metadatos minimos.
 * @param path Path absoluto dentro del sistema de archivos.
 * @param out_inode_number Inodo final resuelto.
 * @param out_mode Modo del inodo final.
 * @param out_size Tamano del inodo final en bytes.
 * @return 1 si el path se resolvio, 0 en caso contrario.
 */
int ext2_resolve_path(const char * path, unsigned int * out_inode_number,
        unsigned short * out_mode, unsigned int * out_size);

/**
 * @brief Lee un archivo regular por path en solo lectura.
 * @param path Path absoluto del archivo.
 * @param buffer Buffer de destino.
 * @param buffer_size Capacidad del buffer.
 * @param out_size Cantidad real de bytes copiados.
 * @return 1 si la lectura fue exitosa, 0 en caso contrario.
 */
int ext2_read_file(const char * path, void * buffer, unsigned int buffer_size,
        unsigned int * out_size);

/**
 * @brief Lista las entradas inmediatas de un directorio en un buffer de texto.
 * @param path Path absoluto del directorio.
 * @param buffer Buffer de salida.
 * @param buffer_size Capacidad del buffer.
 * @param out_size Cantidad real de bytes escritos.
 * @return 1 si el listado fue exitoso, 0 en caso contrario.
 */
int ext2_list_dir(const char * path, void * buffer, unsigned int buffer_size,
        unsigned int * out_size);

#endif /* EXT2_H_ */
