/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 * @brief Contiene la implementación de las rutinas para la gestión del modo
 * protegido IA-32 y de la Tabla Global de Descriptores (GDT)
 */

#include <pm.h>

/** @brief Tabla Global de Descriptores (GDT). Es un arreglo de descriptores
 * de segmento. Según el manual de Intel, esta tabla debe estar alineada a un
 * límite de 8 bytes para un óptimo desempeño. Definida en
 * kernel/src/start.S.*/
extern gdt_descriptor kernel_gdt[];

/** @brief Variable que almacena la siguiente entrada disponible en la GDT */
int current_gdt_entry = 0;

/** @brief Variable que almacena el selector del descriptor de segmento de
 * código del kernel */
unsigned short kernel_code_selector = KERNEL_CODE_SELECTOR;

/** @brief Referencia al Descriptor de segmento de codigo del kernel dentro de
 * la GDT*/
gdt_descriptor * kernel_code_descriptor = &kernel_gdt[KERNEL_CODE_SELECTOR >> 3];

/** @brief  Variable que almacena el selector del descriptor de segmento de
 * datos para el kernel */
unsigned short kernel_data_selector = KERNEL_DATA_SELECTOR;

/** @brief Referencia al Descriptor de segmento de datos del kernel */
gdt_descriptor * kernel_data_descriptor = &kernel_gdt[KERNEL_DATA_SELECTOR >> 3];

/**
 * @brief Función que permite obtener el selector en la GDT a partir de un
 * apuntador a un descriptor de segmento
 * @param desc Referencia al descriptor de segmento del cual se desea obtener
 * el selector
 * @return Selector que referencia al descriptor dentro de la GDT.
 */
unsigned short get_gdt_selector(gdt_descriptor * desc) {
	unsigned short offset = (unsigned short)((char*)desc - (char*)kernel_gdt);
	if (offset < 0) {return 0;}
	if (offset > sizeof(gdt_descriptor) * MAX_GDT_ENTRIES) {return 0;}
	return offset;
}


/**
 * @brief Función que permite obtener el descriptor de segmento en la GDT a
 * partir de un selector
 * @param selector Selector que permite obtener un descriptor de segmento
 * de la GDT
 * @return Referencia al descriptor de segmento dentro de la GDT
 */
gdt_descriptor * get_gdt_descriptor(unsigned short selector) {
	if (selector < 0) {return 0;}
	if (selector > sizeof(gdt_descriptor) * MAX_GDT_ENTRIES) {return 0;}
	if (selector % sizeof(gdt_descriptor)) {
		return 0;
	}
	return &kernel_gdt[selector>>3];
}

/**
 * @brief Permite obtener un descriptor de segmento
 *  disponible en la GDT.
 *  @return Referencia al próximo descriptor de segmento dentro de la GDT que se
 *  encuentra disponible, nulo en caso que no exista una entrada disponible
 *  dentro de la GDT.
 */
gdt_descriptor * allocate_gdt_descriptor(void) {
	unsigned short next_gdt_entry;


	next_gdt_entry = current_gdt_entry;

	do {
			if(next_gdt_entry != 0  /* Valida? */
					&& kernel_gdt[next_gdt_entry].low == 0 /* Entrada vacia? */
					&& 	kernel_gdt[next_gdt_entry].high == 0){
				/*Entrada valida!*/
				/* Marcar la entrada como 'Presente' para evitar
				 * que una llamada concurrente encuentre la misma entrada */
				kernel_gdt[next_gdt_entry].high |= 1 << 15;
				current_gdt_entry = (current_gdt_entry + 1) % MAX_GDT_ENTRIES;
				return &kernel_gdt[next_gdt_entry];
			}
			next_gdt_entry = (next_gdt_entry + 1) % MAX_GDT_ENTRIES;
	}while (next_gdt_entry > current_gdt_entry);
	return 0;
}

/**
 * @brief Buscar un selector disponible dentro de la GDT
 * @return Selector que apunta al descriptor de segmento disponible encontrado
 * dentro de la GDT
 */
unsigned short allocate_gdt_selector(void) {
	gdt_descriptor * desc = allocate_gdt_descriptor();
	if (desc == 0) {return 0;}
	return get_gdt_selector(desc);
}

/**
 * @brief Permite liberar un descriptor de segmento en la GDT.
 * @param desc Apuntador al descriptor que se desa liberar
 * */
void free_gdt_descriptor(gdt_descriptor *desc) {
	unsigned short selector = get_gdt_selector(desc);
	unsigned int index;

	if (selector == 0) {return;}
		index = selector >> 3;
		kernel_gdt[index].low = 0;
		kernel_gdt[index].high = 0;
		current_gdt_entry = index;
}


/**
 * @brief Permite configurar un descriptor de segmento dentro de la GDT
 * @param selector Selector que referencia al descriptor de segmento
 * dentro de la GDT
 * @param base Dirección lineal del inicio del segmento en memoria
 * @param limit Tamaño del segmento
 * @param type Tipo de segmento
 * @param dpl Nivel de privilegios del segmento
 * @param code_or_data 1 = Segmento de código o datos, 0 = segmento del
 * sistema
 * @param opsize Tamaño de operandos: 0 = 16 bits, 1 = 32 bits
 */
void setup_gdt_descriptor(unsigned short selector , unsigned int base,
		unsigned int limit, char type, char dpl, int code_or_data, char opsize)
{

	gdt_descriptor * desc;

	desc = get_gdt_descriptor(selector);

	if (desc == 0) {return;}

	desc->low = (	/* Base 0..15 */
					((base & 0x0000FFFF) << 16) |
					/* limit 0..15 */
					(limit & 0x0000FFFF));

	desc->high = (  /* Base 24..31 */
					(base & 0xFF000000) |
					/* C = (G=1, D/B=?, L=0, AVL=0) */
					(limit & 0x008F0000) |
					((opsize & 0x1) << 22) |
					/* P = 1, S = code_or_data */
					(0x8000 | ((code_or_data & 1) << 12)) |
					((dpl & 0x03) << 13) |
					(type & 0x0F) << 8 |
					((base & 0x00FF0000) >> 16)
					);
}

/**
 * @brief Permite crear un descriptor de idt de 32 bits.
 * @param vector    Numero de la interrupcion
 * @param selector  Selector del GDT a partir del cual se puede determinar
 * 					el segmento de codigo dentro del cual se encuentra
 * 					la rutina de manejo de interrupcion.
 * @param offset	Desplazamiento en el cual se encuentra la rutina de manejo
 * 					de interrupcion dentro del segmento especificado por el
 * 					selector.
 * @param dpl		Nivel de privilegios del descriptor
 * @param type		Tipo de descriptor
 * @returns idt_descriptor : Descriptor de idt creado.
 * */
void setup_idt_descriptor(unsigned char vector, unsigned int selector,
		unsigned int offset, unsigned char dpl, unsigned char type){

    if (vector >= MAX_IDT_ENTRIES) {return;}

    kernel_idt[vector].offset_low =  (unsigned short)(offset & 0x0000FFFF);
    kernel_idt[vector].selector = (unsigned short)(selector & 0x0000FFFF);
    kernel_idt[vector].type = (unsigned short)(((1<<7) | \
                            ((dpl & 0x03) << 5) | (type & 0x0F)) << 8);
    kernel_idt[vector].offset_high = (unsigned short)((offset & 0xFFFF0000) 
                    >> 16);
    

}

