/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 * @brief Contiene la implementación de las rutinas relacionadas
 * con la gestión de memoria física mediante un mapa de bits.
 * La memoria se gestiona en unidades de PAGE_SIZE denominadas marcos de
 * página.
 */
#include <asm.h>
#include <bitmap.h>
#include <console.h>
#include <pm.h>
#include <paging.h>
#include <physmem.h>
#include <multiboot.h>
#include <stdlib.h>

/** @brief Mapa de bits de la memoria fisica. */
unsigned int 
    physical_memory_bitmap[PHYSMEM_MAXFRAMES / BITS_PER_BITMAP_ENTRY] 
    __attribute__((aligned(4096)));

/** @brief Lista de regiones fisicas de memoria */
memory_region physmem[PHYSMEM_REGION_COUNT];

/** @brief Apuntador a la lista de regiones fisicas de memoria */
memory_region * physmem_list = 0;

/** @brief Apuntador a la region fisica de memoria actual */
memory_region * current_physmem;

/** @brief Numero de regiones de memoria disponibles */
int physmem_count;

/* @brief Numero total de marcos de pagina disponibles */
int physmem_available_frames;

/* Variable definida en start.S que almacena la dirección física en la cual
 * terminan las tablas de página iniciales del kernel */
extern unsigned int kernel_initial_pagetables_end;

/** @brief Mapa de bits de memoria disponible
 * @details Esta variable almacena el apuntador del inicio del mapa de bits
 * que permite gestionar las unidades de memoria. */
 unsigned int * memory_bitmap;

/** @brief Variable global del kernel que almacena el inicio de la región
 * de memoria disponible */
unsigned int memory_start;
/** @brief Variable global del kernel que almacena el tamano en bytes de
 * la memoria disponible */
unsigned int memory_length;

/** @brief Mínima dirección de memoria permitida para liberar */
unsigned int allowed_free_start;

/* @brief Variable que almacena la ubicación de la estructura multiboot en
 * memoria. Definida en start.S */
extern unsigned int multiboot_info_location;

/**
 * @brief Inicializa el mapa de bits de memoria, a partir de la informacion
 * proporcionada por GRUB.
 */
void setup_physical_memory(void){

    int i;

	extern multiboot_header_t multiboot_header;

    /* Apuntar al mapa de bits de memoria física. */
    memory_bitmap = (unsigned int*)&physical_memory_bitmap;

	/* Variables temporales para hallar la region de memoria disponible */
	unsigned int tmp_start;
	unsigned int tmp_length;
	unsigned int tmp_end;
	int mod_count;
    unsigned int mmap_address;
    unsigned int mods_address;

    unsigned int * tmp_ptr;
    int slots;

    /* Dado que ya se habilitó la memoria virtual, se debe usar la
     * dirección virtual en la cual se encuentra mapeada la estructura de
     * información multiboot. */
	multiboot_info_t * info = (multiboot_info_t *)(multiboot_info_location 
            + KERNEL_VIRT_OFFSET);

    /* Almacena la dirección de memoria final del ultimo modulo cargado, o
     * 0 si no se cargaron modulos. */
	unsigned int mods_end; 

	/* si flags[3] = 1, se especificaron módulos que deben ser cargados junto
	 * con el kernel y justo después del mismo. */
	mods_end = 0;

    /* Se debe sumar KERNEL_VIRT_OFFSET a la dirección, dado que ya se
     * activó la memoria virtual. */
	if (test_bit(info->flags, 3)) {
		mod_info_t * mod_info;
        mods_address = info->mods_addr + KERNEL_VIRT_OFFSET;
		for (mod_info = (mod_info_t*)(mods_address), mod_count=0;
				mod_count < info->mods_count;
				mod_count++, mod_info++) {
			if (mod_info->mod_end > mods_end) {
				/* Los modulos se redondean a limites de 4 KB, redondear
				 * la dirección final del modulo a un limite de 4096 */
				mods_end = mod_info->mod_end + (mod_info->mod_end % 4096);
			}
		}
	}


	/* si flags[6] = 1, los campos mmap_length y mmap_addr son validos */

	/* Revisar las regiones de memoria, y extraer la region de memoria
	 * de mayor tamano, marcada como disponible, cuya dirección base sea
	* mayor o igual a la posicion del kernel en memoria.
	*/

	memory_start = 0;
	memory_length = 0;

    physmem_count = 0;
    physmem_available_frames = 0;

	/* Suponer que el inicio de la memoria disponible se encuentra
	 * al finalizar el kernel, los módulos, el directorio de tablas de página y
     * las tablas de página del kernel. */
	allowed_free_start = kernel_initial_pagetables_end;

	/** Existe un mapa de memoria válido creado por GRUB? */
	if (test_bit(info->flags, 6)) {
        
		memory_map_t *mmap;

        /* Calcular la dirección virtual del mapa de memoria*/
        mmap_address = info->mmap_addr + KERNEL_VIRT_OFFSET;

		for (mmap = (memory_map_t *) (mmap_address);
			(unsigned int) mmap < mmap_address +  info->mmap_length;
			mmap = (memory_map_t *) ((unsigned int) mmap
									 + mmap->entry_size
									 + sizeof (mmap->entry_size))) {

	  /** Verificar si la región de memoria cumple con las condiciones
	   * para ser considerada "memoria disponible".
	   *
	   * Importante: Si se supone un procesador de 32 bits, los valores
	   * de la parte alta de base y length (base_addr_high y
	   * length_high) son cero. Por esta razon se pueden ignorar y solo
	   * se usan los 32 bits menos significativos de base y length.
	   *
	   * Para que una region de memoria sea considerada "memoria
	   * disponible", debe cumplir con las siguientes condiciones:
	   *
	   * - Estar ubicada en una posicion de memoria mayor o igual que
	   * 	1 MB.
	   * - Tener su atributo 'type' en 1 = memoria disponible.
	   * */
		 /* La region esta marcada como disponible y su dirección base
		  * es igual o superior a 1 MB? */
		 if (mmap->type == 1 && mmap->base_addr_low >= 0x100000)  {
			 tmp_start = mmap->base_addr_low;
			 tmp_length = mmap->length_low;

			 /* Verificar si el kernel se encuentra en esta region */
			 if (kernel_initial_pagetables_end >= tmp_start &&
					kernel_initial_pagetables_end <= tmp_start + tmp_length) {
				 /* El kernel se encuentra en esta region. Tomar el inicio
				  * de la memoria disponible en la posicion en la cual
				  * finaliza el kernel
				 */
				 tmp_start =kernel_initial_pagetables_end;

				 /* Ahora verificar si ser cargaron módulos junto con el
				  * kernel. Estos modulos se cargan en regiones continuas
				  * al kernel.
				  * Si es así, la nueva posición inicial de la memoria
				  * disponible es la posición en la cual terminan los módulos
				  * */
				 if (mods_end > 0 &&
						 mods_end >= tmp_start &&
						 mods_end <= tmp_start + tmp_length) {
						tmp_start = mods_end;
				 }
				 /* Restar al espacio disponible.*/
				 tmp_length -= tmp_start - mmap->base_addr_low;
				 if (tmp_length > memory_length) {
					 memory_start = tmp_start;
					 memory_length = tmp_length; /* Tomar el espacio */
				 }
			 }else {
				 /* El kernel no se encuentra en esta region, verificar si
				  * su tamano es mayor que la region mas grande encontrada
				  * hasta ahora
				  */
				 if (tmp_length > memory_length) {
					 memory_start = tmp_start;
					 memory_length = tmp_length; /* Tomar el espacio */
				 }
			 }
		 }
		} //endfor
	}

	/* Existe una región de memoria disponible? */
	if (memory_start > 0 && memory_length > 0) {

        /*
        console_printf("Memory start at: 0x%x, length:0x%x\n", 
                memory_start, memory_length);
        */

		/* Antes de retornar, establecer la minima dirección de memoria
		 * permitida para liberar*/

		tmp_start = memory_start;
		/* Calcular la dirección en la cual finaliza la memoria disponible */
		tmp_end = tmp_start + tmp_length;

		/* Redondear el inicio y el fin de la región de memoria disponible a
         * páginas */
		tmp_end = ROUND_DOWN_TO_PAGE(tmp_end);
		tmp_start = ROUND_UP_TO_PAGE(tmp_start);

		/* Calcular el tamano de la región de memoria disponible, redondeada
		 * a límites de página */
		tmp_length = tmp_end - tmp_start;

		/* Actualizar las variables globales del kernel */
		memory_start = tmp_start;
		memory_length = tmp_length;

        /* Establecer la dirección de memoria a partir de la cual se puede
         * liberar memoria */
		allowed_free_start = memory_start;

        /* Inicializar las regiones de memoria disponibles */

        tmp_start = memory_start; //Inicio de la memoria fisica disponible
        tmp_end = memory_start + memory_length; //Fin de la memoria fisica
        tmp_ptr = (unsigned int*)&physical_memory_bitmap; //Mapa de bits

        do {
            if (tmp_start < tmp_end) {
                physmem[physmem_count].start = tmp_start;
                physmem[physmem_count].next = 0;
                physmem[physmem_count].prev = 0;
                if (physmem_count > 0) {
                    physmem[physmem_count].prev = &physmem[physmem_count - 1];
                    physmem[physmem_count -1].next = &physmem[physmem_count];
                }
                if (tmp_start + PHYSMEM_GRANULARITY < tmp_end) {
                    physmem[physmem_count].length = 
                        PHYSMEM_GRANULARITY;
                }else {
                    physmem[physmem_count].length = tmp_end - tmp_start;
                }

                slots = physmem[physmem_count].length / PAGE_SIZE;

                /* Inicializar el mapa de bits para esta region */
                bitmap_init(&physmem[physmem_count].map, tmp_ptr, slots);
                tmp_ptr += slots / BITS_PER_BITMAP_ENTRY;

                /* Redondear al siguiente apuntador de entero sin signo si
                 * es necesario */
                if (slots % BITS_PER_BITMAP_ENTRY != 0) {
                    tmp_ptr++;
                }

                /*
                console_printf("0x%x (%d)\n", 
                        physmem[physmem_count].start,
                        physmem[physmem_count].map.free_slots);
                */

                physmem_count++;
                physmem_available_frames += slots;
            }
            tmp_start += PHYSMEM_GRANULARITY;
        }while (tmp_start < tmp_end);

        if (physmem_count > 0) {
            physmem[physmem_count - 1].next = &physmem[0];
            physmem[0].prev = &physmem[physmem_count - 1];
            physmem_list = &physmem[0];
            current_physmem = physmem_list;
        }
    }
}

/**
 @brief Reserva un marco libre dentro del mapa de bits de memoria.
 * @return Dirección de inicio del marco de página, 0 si no existen marcos
 * disponibles
 */
unsigned int allocate_frame() {
    unsigned int addr;
    int slot;
    memory_region * aux;

    aux = current_physmem;
    
    if (physmem_available_frames == 0) {
        return 0;
    }

    do {
        if (aux->map.free_slots != 0) {
            slot = bitmap_allocate(&aux->map);
            if (slot >= 0) {
                addr = aux->start + (slot * PAGE_SIZE);
                physmem_available_frames--;
                return addr;
            }
        }
        aux = aux->next;
    }while(aux != current_physmem);
    
    return 0;
}

/** @brief Reserva una región de memoria contigua libre dentro del mapa de bits
* de memoria.
* @param length Tamano de la región de memoria a asignar.
* @return Dirección de inicio de la región en memoria, 0 si no es posible
* reservar.
*/
unsigned int allocate_frame_region(unsigned int length) {
	unsigned int frame_count;
    unsigned int addr;
    int slot;
    memory_region * aux;

    frame_count = (length / PAGE_SIZE);

	if (length % PAGE_SIZE > 0) {
		frame_count++;
	}

    if (physmem_available_frames < frame_count) {
        return 0;
    }

    aux = current_physmem;

    do {
        if (aux->map.free_slots != 0 && 
                aux->map.free_slots >= frame_count) {
            slot = bitmap_allocate_region(&aux->map, frame_count);
            if (slot >= 0) {
                addr = aux->start + (slot * PAGE_SIZE);
                physmem_available_frames -= frame_count;
                return addr;
            }
        }
        aux = aux->next;
    }while(aux != current_physmem);
    
    return 0;
}

/**
 * @brief Permite liberar un marco de página
 * @param addr Dirección de inicio del marco. Se redondea hacia abajo si no es
 * múltiplo de PAGE_SIZE
 */
void free_frame(unsigned int addr) {
    int slot;
    unsigned int start;
    memory_region * aux;

    start = ROUND_DOWN_TO_PAGE(addr);
    aux = current_physmem;

    do {
        if (start >= aux->start && start < aux->start + aux->length) {
            slot = (start - aux->start) / PAGE_SIZE;
            bitmap_free(&aux->map, slot);
            physmem_available_frames++;
            return;
        }
        aux = aux->next;
    }while(aux != current_physmem);
    //console_printf("Frame at 0x%x not found!\n");
}

/**
 * @brief Retorna el número de marcos de mágina libres
 * @return Número de marcos de página disponibles
 */
int available_frames() {
    return physmem_available_frames;
}


