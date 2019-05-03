/** 
   @file
   @author Erwin Meza <emezav@gmail.com> 
   @brief Archivo  para realizar el enlace (linking) del bootsector.
   El linker toma el codigo de objeto y organiza el archivo ejecutable
   final. Para ello, utiliza este archivo de configuracion, en el cual
   se especifica el formato del archivo ejecutable resultante.   
*/
/** @verbatim */

ENTRY(start); /* Punto de entrada: etiqueta start en bootsect.S */
OUTPUT_FORMAT(binary); /* Formato del archivo de salida: binario */
phys = 0x0; /* dirección relativa al inicio del bootsector*/

/* Organizacion del archivo binario de salida */
SECTIONS {
   . = phys;  
   .text : { /* Codigo */
     /* Incluir la seccion de texto de todos los archivos de entrada */
     *(.text)		  

    /* Asegurarse que el codigo del bootsector ocupa exactamente 510 bytes 
       hasta este punto
    */
     . = phys + 510;	

     /* Firma del bootsector: Para que la BIOS considere el sector de arranque
        como valido, sus dos ultimos bytes deben ser 0x55 y 0xAA. (el numero
        0xAA55 en little-endian) */
     BYTE (0x55)
     BYTE (0xAA)
   } = 0x90 /* Rellenar con '0x90' (no-op), desde el final 
   del codigo hasta completar los 510 bytes del bootsector */

   /* Ignorar estas secciones de todos los archivos .o, ya que el codigo
      del bootsector se encuentra en la seccion .text */
   /DISCARD/ : { 
	   *(.data)
	   *(.rodata)
	   *(.bss)
   }
}

/** @endverbatim */