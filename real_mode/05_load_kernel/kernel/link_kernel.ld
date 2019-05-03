/** 
   @file
   @author Erwin Meza <emezav@gmail.com> 
   @brief Archivo  para realizar el enlace (linking) del kernel.
   Este archivo toma los archivos de objeto y organiza el archivo
   final. 
*/
/** @verbatim */
/* 
   Se supone que el kernel se carga en la dirección de memoria 0x8000.
*/

ENTRY(kernel_start);
OUTPUT_FORMAT(binary);
phys = 0x0000;
virt = 0x0000;
SECTIONS {
   . = phys;
   kernel_start = .;
   .text : AT(virt) {
     code_start = .;
	 /* Asegurarse que el código de start.o se copie primero */
	 *(.bootcode)
     *(.text)
     *(.rodata) /* Agregar la seccion 'rodata' de todos los archivos */
     . = ALIGN(4096);
     code_end = .;
   } = 0x90909090
   .data : AT (virt + (data_start - code_start)) {
       data_start = .;
       *(.data)
     . = ALIGN(4096);
       data_end = .;
   } = 0x00000000
   .bss : AT (phys + (bss_start - code_start)) {
       bss_start = .;
       *(.bss)
       *(COMMON)
     . = ALIGN(4096);
       bss_end = .;
   } = 0x00000000
   
   kernel_end = .;
   
   /* Sectores efectivos desde el fin del segmento de datos hasta el inicio del 
      segmento  de codigo */

   kernel_sectors = ( (data_end - code_start) / 512); 
   /* El simbolo kernel_sectors almacena el numero de sectores
      que ocupa el kernel. 
      El valor de este simbolo se copia en el byte 4 del archivo binario del
      kernel (ver start.S)
   */
      
}

/** @endverbatim */
