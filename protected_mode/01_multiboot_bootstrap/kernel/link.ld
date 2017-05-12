/** 
   @file
   @author Erwin Meza <emezav@gmail.com> 
   @brief Archivo  para realizar el enlace (linking) del kernel.
   Este archivo toma los archivos de objeto y organiza el archivo
   final. 
*/
/** @verbatim */
/* 
   Se supone que el kernel se carga en la dirección de memoria 0x100000.
*/

/* Dirección física en la cual se carga el kernel en memoria */
phys = 0x100000;

ENTRY(start);
OUTPUT_FORMAT(elf32-i386);
SECTIONS {
   . = phys;
	/* dirección física en la cual se carga el kernel */
	kernel_phys_start = .;
	/* Segmento de texto */
   .text ALIGN (0x1000): AT(code_start) {
    /* Inicio de la sección de código */
     code_start = .;	
    /* Incluir primero la sección de código de start.S llamada .startcode */
     *(.startcode) 
     *(.text)
     *(.rodata)
   } = 0x90909090
   kernel_code_phys_end = .;
   .data ALIGN(0x1000) : AT (data_start) {
       data_start = .;
    /* Incluir primero la sección de código de start.S llamada .startcode */
	   *(.startdata) 
       *(.data)
   } = 0x00000000
   data_phys_end = .;
   .bss : AT (bss_start) {
       bss_start = .;
       *(.bss)
       *(COMMON)
   } = 0x00000000

   . = ALIGN(4096);

   bss_phys_end = .;

}

/** @endverbatim */
