/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 * @brief Contiene la implementación de las rutinas relacionadas
 * con la gestión de memoria virtual del kernel mediante un mapa de bits.
 */
#include <stdlib.h>
#include <kmem.h>

/** @brief Mapa de bits de la memoria lineal del kernel. */
unsigned int 
    kernel_memory_bitmap[KMEM_MAXPAGES / BITS_PER_BITMAP_ENTRY] 
    __attribute__((aligned(4096)));

/** @brief Lista de regiones de memoria virtual */
memory_region kmem[KMEM_REGION_COUNT];

/** @brief Apuntador a la lista de regiones de memoria */
memory_region * kmem_list = 0;

/** @brief Apuntador a la region de memoria actual */
memory_region * current_kmem;

/** @brief Numero de regiones de memoria disponibles */
int kmem_count;

/** @brief Numero total de paginas disponibles */
int kmem_available_pages;

/* Variable definida en start.S que almacena la dirección física en la cual
 * terminan las tablas de página iniciales del kernel */
extern unsigned int kernel_initial_pagetables_end;

/** @brief Mapa de bits de memoria virtual del kernel
 * @details Esta variable almacena el apuntador del inicio del mapa de bits
 * que permite gestionar las unidades de memoria. */
 unsigned int * kmem_bitmap;

/**
 * @brief Inicializa la memoria virtual del kernel
 */

void setup_kmem(void){

    int i;
    unsigned int tmp_start;
    unsigned int tmp_end;

    /* Apuntar al mapa de bits de la memoria del kernel. */
    kmem_bitmap = (unsigned int*)&kernel_memory_bitmap;

    unsigned int * tmp_ptr;
    int slots;

	//kernel_initial_pagetables_end;

    /* Inicializar las regiones de memoria disponibles */
    kmem_count = 0;
    kmem_available_pages = 0;

    //Inicio de la memoria virtual disponible
    //Donde terminan las tablas de pagina  iniciales + 1 pagina
    tmp_start = kernel_initial_pagetables_end + PAGE_SIZE + KERNEL_VIRT_OFFSET; 

    /*console_printf("Available virtual memory starts at 0x%x\n", tmp_start);*/

    //Fin de la memoria virtual disponible
    tmp_end = KMEM_LIMIT - KMEM_RESERVED;

    //Mapa de bits
    tmp_ptr = (unsigned int*)&kernel_memory_bitmap; 

    do {
        if (tmp_start < tmp_end) {
            kmem[kmem_count].start = tmp_start;
            kmem[kmem_count].next = 0;
            kmem[kmem_count].prev = 0;
            if (kmem_count > 0) {
                kmem[kmem_count].prev = &kmem[kmem_count - 1];
                kmem[kmem_count -1].next = &kmem[kmem_count];
            }
            if (tmp_start + KMEM_GRANULARITY < tmp_end) {
                kmem[kmem_count].length = 
                    KMEM_GRANULARITY;
            }else {
                kmem[kmem_count].length = tmp_end - tmp_start;
            }

            slots = kmem[kmem_count].length / PAGE_SIZE;
            kmem_available_pages += slots;

            /* Inicializar el mapa de bits para esta region */
            bitmap_init(&kmem[kmem_count].map, tmp_ptr, slots);

            /* Apuntar a la siguiente entrada */
            tmp_ptr += slots / BITS_PER_BITMAP_ENTRY;

            /* Redondear al siguiente apuntador de entero sin signo si
                * es necesario */
            if (slots % BITS_PER_BITMAP_ENTRY != 0) {
                tmp_ptr++;
            }

            
            /*
            console_printf("0x%x (%d)\n", 
                    kmem[kmem_count].start,
                    kmem[kmem_count].map.free_slots);
            */
            

            kmem_count++;
        }
        tmp_start += KMEM_GRANULARITY;
    }while (tmp_start < tmp_end);

    if (kmem_count > 0) {
        kmem[kmem_count - 1].next = &kmem[0];
        kmem[0].prev = &kmem[kmem_count - 1];
        kmem_list = &kmem[0];
        current_kmem = kmem_list;
    }
}

/**
 * @brief Busca y reserva una página libre dentro de la memoria del kernel
 * @return Dirección de inicio de la página
 */
unsigned int kmem_get_page() {
    unsigned int addr;
    int slot;
    memory_region * aux;

    if (kmem_available_pages == 0) {
        return 0;
    }

    aux = current_kmem;

    do {
        if (aux->map.free_slots != 0) {
            slot = bitmap_allocate(&aux->map);
            if (slot >= 0) {
                addr = aux->start + (slot * PAGE_SIZE);
                kmem_available_pages--;
                return addr;
            }
        }
        aux = aux->next;
    }while(aux != current_kmem);
    
    return 0;
}

/**
 * @brief Busca una región continua de páginas libres en la memoria del kernel
 * @return Dirección de inicio de la página
 */
unsigned int kmem_get_pages(int count) {
    unsigned int addr;
    int slot;
    memory_region * aux;

    aux = current_kmem;

    if (kmem_available_pages < count) {
        return 0;
    }

    do {
        if (aux->map.free_slots != 0 && 
                aux->map.free_slots >= count) {
            slot = bitmap_allocate_region(&aux->map, count);
            if (slot >= 0) {
                addr = aux->start + (slot * PAGE_SIZE);
                kmem_available_pages -= count;
                return addr;
            }
        }
        aux = aux->next;
    }while(aux != current_kmem);
    
    return 0;
}

/**
 * @brief Busca una página y un marco libre y realiza el mapeo
 * @return Dirección de inicio de la página
 */
unsigned int kmem_allocate_page(void){
    unsigned int frame;
    unsigned int page;

    frame = allocate_frame();
    /*console_printf("Frame at: 0x%x\n", frame);*/
    if (!frame) {
        return 0;
    }

    page = kmem_get_page();
    /*console_printf("Page at: 0x%x\n", page);*/

    if (!page) {
        free_frame(frame);
        return 0;
    }

    if (map_page(page, frame)) {
        return page;
    }else {
        kmem_free(page);
        //No se pudo mapear la pagina, liberar el marco
        free_frame(frame);
    }
    return 0;
}

/**
 * @brief Busca y mapea una región continua de páginas libres
 * @param count Numero de paginas a buscar y mapear
 * @param adjacent 1 = los marcos deben ser contiguos en memoria fisica
 * @return Dirección de inicio de la página
 */
unsigned int kmem_allocate_pages(int count, int adjacent) {
    int i;
    unsigned int page;
    int done;


    unsigned int tmp_frame;
    unsigned int tmp_page;
    int j;
    unsigned int frame;

    //Verificar si existen suficientes marcos de pagina disponibles
    if (available_frames() < count) {
        return 0;
    }

    //Obtener las paginas contiguas en memoria virtual
    page = kmem_get_pages(count);
    if (!page) {
        return 0;
    }

    tmp_page = page;
    done = 1;

    if (!adjacent) {
        //Mapear las paginas a marcos (no necesariamente adyacentes)
        for (i = 0 ;
                i < count && done == 1; 
                i++, tmp_page += PAGE_SIZE){
            tmp_frame = allocate_frame();
            if (!map_page(tmp_page, tmp_frame)) {
                done = 0;
            }
        }

    }else {
        //Buscar count marcos de pagina adyacentes
        frame = allocate_frame_region(count * FRAME_SIZE);
        if (frame) {
            tmp_frame = frame;
            done = 1;
            for (i = 0; 
                    i < count && done == 1; 
                    i++, tmp_page += PAGE_SIZE, tmp_frame += FRAME_SIZE){
                if (!map_page(tmp_page, tmp_frame)) {
                    done = 0;
                }
            }
        }else {
            done = 0;
        }
    }
    if (done) {
        return page;
    }else {
        for (j = 0; j <i; j++, page += PAGE_SIZE) {
            kmem_free(page);
        }
    }
    return 0;
}


/**
 * @brief Permite liberar una página
 * @param addr Dirección de la página a liberar
 */
void kmem_free(unsigned int addr) {
    int slot;
    unsigned int start;
    memory_region * aux;

    start = ROUND_DOWN_TO_PAGE(addr);
    aux = current_kmem;

    do {
        if (start >= aux->start && start < aux->start + aux->length) {
            slot = (start - aux->start) / PAGE_SIZE;
            bitmap_free(&aux->map, slot);
            //Liberar la pagina y el marco de pagina
            kmem_available_pages++;
            destroy_page(addr);
            return;
        }
        aux = aux->next;
    }while(aux != current_kmem);
}

/**
 * @brief Retorna el número de páginas disponibles en la memoria del kernel
 * @return Número de páginas disponibles
 */
int available_pages() {
    return kmem_available_pages;
}


