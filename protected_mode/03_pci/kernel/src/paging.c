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

#include <pm.h>
#include <asm.h>
#include <exception.h>
#include <paging.h>
#include <console.h>
#include <stdlib.h>
#include <physmem.h>

/* @brief Apuntador al inicio del directorio de tablas de página */
page_directory kernel_pd;

/**
 * @brief Completa el proceso de configurar la paginación para el kernel.
 */
void setup_paging() {
    int i;
    unsigned int new_frame;

    extern unsigned int kernel_initial_pagetables_end;
    extern unsigned int kernel_page_tables;


    /* Usar la dirección virtual del directorio de tablas de página */
    kernel_pd = (unsigned int *)(kernel_pd_addr + KERNEL_VIRT_OFFSET);

    /* Mapear el directorio de tablas de página de forma recursiva.
     * La última entrada del directorio de tablas de página contendrá la
     * dirección física del mismo directorio de tabla de páginas.
     *
     * Las tablas de página (1023 de las 1024 posibles en un espacio virtual de
     * 4 GB) se mapean al final del espacio virtual de 4 GB en la dirección
     * KERNEL_PAGETABLES_VADDR
     *
     * La última tabla de páginas es el mismo directorio de tablas de página. De
     * esta forma, el directorio de tablas de página se usa también como una
     * tabla de páginas para acceder a los últimos 4 MB de la memoria virtual,
     * es decir, a todas las tablas de páginas.
     * */
    kernel_pd[PD_ENTRIES - 1] = (kernel_pd_addr | PG_KERNEL_PRESENT);

    /* Luego de mapear el directorio de tablas de página de forma recursiva, lo
     * podemos acceder en la dirección virtual KERNEL_PD_VADDR */
    kernel_pd = (unsigned int *)(KERNEL_PD_VADDR);
    
    /* Ya podemos abandonar completamente el espacio físico!
     * El kernel seguirá en su posición en memoria física, pero se ejecutará
     * completamente en memoria virtual. Por tal razón, se debe quitar el mapeo
     * 1:1 del primer MB y el kernel que se necesitaba para poder activar la
     * paginación.
     */

    /* Eliminar las entradas del directorio de tablas de página que mapean el
     * kernel en la parte baja de la memoria */
    for (i = 0; i < kernel_page_tables; i++) {
        kernel_pd[i] = PG_UNUSED;
    }

    /* Se deben invalidar las entradas del TLB de todas las páginas mapeadas 1:1
     * en la parte baja de la memoria desde la dirección 0 hasta el final del
     * kernel el directorio de tablas de página y las páginas iniciales. */
    for (i = 0; i< kernel_initial_pagetables_end; i = i + PAGE_SIZE) {
        invalidate_page(i);
    }

    /* A partir de este momento cualquier referencia a una página no mapeada
     * generará una excepción de generará un fallo de página, y se
     * ejecutará la subrutina page_fault_hadler. */

    /* Instalar el manejador de excepción de fallo de página */
    install_exception_handler(PAGE_FAULT_EXCEPTION, page_fault_handler);
}

/**
 * @brief Crea una nueva tabla de páginas vacía y retorna su dirección física.
 * Retorna 0 si no es posible crear la nueva tabla de páginas, dado que jamás se
 * podrá usar el marco 0 de la memoria física. 
 */
unsigned int create_new_page_table(int pd_entry) {
    unsigned int frame_addr;
    int i;
    page_table pt;

    /* Obtener un marco de página */
    frame_addr = allocate_frame();

    //console_printf("Frame for page table allocated at 0x%x\n", frame_addr);

    /* No hay memoria disponible? Retornar! */
    if (!frame_addr) {
        return 0;
    }

    /* Marcar la entrada del directorio como válida, con lo cual automáticamente
     * se tiene acceso a la tabla de páginas en el espacio virtual a partir de
     * KERNEL_PAGETABLES_VADDR */
    kernel_pd[pd_entry] = frame_addr | PG_KERNEL_PRESENT;

    //console_printf("Entry: %d Page table at: 0x%x\n", pd_entry, (unsigned int)pt);

    /* Inicializa todas las entradas de la nueva tabla de páginas */
    pt = (page_table)(KERNEL_PAGETABLES_VADDR + (pd_entry * PAGE_SIZE));
    for (i = 0; i < PT_ENTRIES; i++) {
        pt[i] = PG_UNUSED;
    }

    return frame_addr;
    
}

/**
 * @brief Permite mapear una página a un marco de página en el espacio virtual
 * Retorna 1 si se mapeó correctamente 0, en caso de error
 */
int map_page(unsigned int vaddr, unsigned int addr) {
    int pd_entry;
    int pt_entry;
    unsigned int new_addr;
    page_table pt;

    /* Redondear la dirección virtual a un límite de página */
    vaddr = ROUND_DOWN_TO_PAGE(vaddr);

    /* No se puede tocar los últimos 4 MB de la memoria*/
    if (vaddr >= KERNEL_PAGETABLES_VADDR) {
        /* No se puede mapear una nueva página en la región destinada para las
         * tablas de página en la memoria virtual  */
        //console_printf("Warning! attempting to map over the page tables memory!\n");
        return 0;
    }

    /* Obtener la entrada en el directorio y en la tabla de páginas */
    pd_entry = vaddr / (PAGE_SIZE * PD_ENTRIES);
    pt_entry = (vaddr % (PAGE_SIZE * PD_ENTRIES)) / PAGE_SIZE;

    //console_printf("PD entry: %d PT entry: %d\n", pd_entry, pt_entry);

    /* Si la tabla de páginas no se encuentra presente, reservar memoria para la
     * tabla de páginas, inicializar y registrar en el directorio */ 
    if (! (kernel_pd[pd_entry] & PG_PRESENT)) {
        if (! (new_addr = create_new_page_table(pd_entry))) {
            //No se pudo crear la tabla de páginas, retornar.
            // console_printf("Could not create page table for entry %d\n", pd_entry);
            return 0;
        }
    }

    /* Ubicar la tabla de páginas correspondiente y marcar la entrada como 
     * usada */
    pt = (page_table)((KERNEL_PAGETABLES_VADDR) + (pd_entry * PAGE_SIZE));
    if (pt[pt_entry] & PG_PRESENT) {
        //La página ya está mapeada a algún marco en memoria! 
        //console_printf("Warning! attempting to map an already mapped page!\n");
        return 0;
    }

    /* Actualizar la entrada en la tabla de páginas con la dirección física
     * correspondiente. */
    pt[pt_entry] = addr | PG_KERNEL_PRESENT;

    return 1;
}

/**
 * @brief Permite quitar una página del espacio virtual
 */
int unmap_page(unsigned int vaddr) {
    int pd_entry;
    int pt_entry;
    int i;
    page_table pt;
    unsigned int pt_frame;

    /* Redondear la dirección virtual a un límite de página */
    vaddr = ROUND_DOWN_TO_PAGE(vaddr);

    /* No se puede tocar los últimos 4 MB de la memoria*/
    if (vaddr >= KERNEL_PAGETABLES_VADDR) {
        /* No se puede quitar una nueva página en la región destinada para las
         * tablas de página en la memoria virtual  */
        //console_printf("Warning! attempting to unmap over the page tables memory!\n");
        return 0;
    }

    /* Obtener la entrada en el directorio y en la tabla de páginas */
    pd_entry = vaddr / (PAGE_SIZE * PD_ENTRIES);
    pt_entry = (vaddr % (PAGE_SIZE * PD_ENTRIES)) / PAGE_SIZE;

    //console_printf("PD entry: %d PT entry: %d\n", pd_entry, pt_entry);

    /* No se puede quitar el mapeo de una tabla que no está presente. */
    if (! (kernel_pd[pd_entry] & PG_PRESENT)) {
        //console_printf("Warning! PD entry %d not present!\n", pd_entry);
        return 0;
    }

    /* Ubicar la tabla de páginas correspondiente y marcar la entrada como
     * usada */
    pt = (page_table)((KERNEL_PAGETABLES_VADDR) + (pd_entry * PAGE_SIZE));
    if (pt[pt_entry] & PG_PRESENT) {
        pt[pt_entry] = PG_UNUSED;
    }

    /* Invalidar la página en el TLB */
    invalidate_page(vaddr);

    /* Recorrer la tabla de páginas para verificar si está usando alguna entrada
     * */
    i = 0;
    while (i < PT_ENTRIES && !(pt[i] & PG_PRESENT)){
        i++;
    }
    //console_printf("Page table empty entries: %d\n", i);

    /* Si ninguna entrada de la tabla está siendo usada, se puede liberar la
     * página de memoria que contiene la tabla de páginas y marcar la entrada
     * correspondiente en el directorio como no usada. */
    if (i == 1024) {
        /* Obtener la dirección del marco de página en el cual se encuentra la
         * tabla de páginas */
        pt_frame = kernel_pd[pd_entry] & 0xFFFFF000;

        //console_printf("Invalidate page 0x%x => 0x%x\n", (unsigned int)pt, pt_frame);
        
        /* Invalidar la página en el TLB */
        invalidate_page((unsigned int)pt);

        /* Marcar la entrada en el directorio como no usada */
        kernel_pd[pd_entry] = PG_UNUSED;

        /* Liberar el marco de página que tenía asignada la tabla */
        free_frame(pt_frame);
    }
    return 1;
}

/**
 * @brief Quita una página del espacio virtual y libera el marco asociado
 */
int destroy_page(unsigned int vaddr) {
    int pd_entry;
    int pt_entry;
    int i;
    page_table pt;
    unsigned int frame;
    unsigned int pt_frame;

    /* Redondear la dirección virtual a un límite de página */
    vaddr = ROUND_DOWN_TO_PAGE(vaddr);

    /* No se puede tocar los últimos 4 MB de la memoria*/
    if (vaddr >= KERNEL_PAGETABLES_VADDR) {
        /* No se puede quitar una nueva página en la región destinada para las
         * tablas de página en la memoria virtual  */
        //console_printf("Warning! attempting to unmap over the page tables memory!\n");
        return 0;
    }

    /* Obtener la entrada en el directorio y en la tabla de páginas */
    pd_entry = vaddr / (PAGE_SIZE * PD_ENTRIES);
    pt_entry = (vaddr % (PAGE_SIZE * PD_ENTRIES)) / PAGE_SIZE;

    //console_printf("\nVaddr: 0x%x PD entry: %d PT entry: %d\n", vaddr, pd_entry, pt_entry);

    /* No se puede quitar el mapeo de una tabla que no está presente. */
    if (! (kernel_pd[pd_entry] & PG_PRESENT)) {
        //console_printf("Warning! PD entry %d not present!\n", pd_entry);
        return 0;
    }

    /* Ubicar la tabla de páginas correspondiente y marcar la entrada como no
     * usada */
    pt = (page_table)((KERNEL_PAGETABLES_VADDR) + (pd_entry * PAGE_SIZE));
    if (pt[pt_entry] & PG_PRESENT) {
        frame = pt[pt_entry] & 0xFFFFF000;
        //Libera el marco de pagina de la memoria fisica
        free_frame(frame);
        pt[pt_entry] = PG_UNUSED;
    }

    /* Invalidar la página en el TLB */
    invalidate_page(vaddr);

    /* Recorrer la tabla de páginas para verificar si está usando alguna entrada
     * */
    i = 0;
    while (i < PT_ENTRIES && !(pt[i] & PG_PRESENT)){
        i++;
    }

    //console_printf("Page table empty entries: %d\n", i);

    /* Si ninguna entrada de la tabla está siendo usada, se puede liberar la
     * página de memoria que contiene la tabla de páginas y marcar la entrada
     * correspondiente en el directorio como no usada. */
    if (i == 1024) {
        /* Obtener la dirección del marco de página en el cual se encuentra la
         * tabla de páginas */
        pt_frame = kernel_pd[pd_entry] & 0xFFFFF000;

        //console_printf("Invalidate page table %d => 0x%x\n", pd_entry, pt_frame);
        
        /* Invalidar la página en el TLB */
        invalidate_page((unsigned int)pt);

        /* Marcar la entrada en el directorio como no usada */
        kernel_pd[pd_entry] = PG_UNUSED;

        /* Liberar el marco de página que tenía asignada la tabla */
        free_frame(pt_frame);
    }
    return 1;
}

/**
 * @brief Imprime las entradas presentes de una tabla de páginas
 */
void print_page_table(page_directory pd) {
    int i;

    
    console_printf("Page table at 0x%x[0x%x]\n", (unsigned int)pd,
            ROUND_DOWN_TO_PAGE((unsigned int)pd[1023]));
    
    
     /* Imprimir las entradas marcadas como válidas en el directorio de tablas
      * de  página */
    for (i = 0; i < PD_ENTRIES; i++) {
        if (pd[i] & PG_PRESENT) {
            console_printf("PD[%d] = 0x%x\n", i, pd[i]);
        }
    }
}

/* @brief Obtiene el valor del registro CR2.
 *  @return unsigned int */
unsigned int read_cr2() {
    unsigned int result;
    inline_assembly("mov %%cr2, %%eax\n\t"
                    "mov %%eax, %0\n\t": "=m"(result)::"%eax");
    return result;
}

/* @brief Rutina básica para manejar un fallo de página.
 * Bloquea el kernel si existe fallo de página. */
void page_fault_handler(interrupt_state * state) {
    unsigned int vaddr;
    unsigned int page;

    /* Se debe mapear la página a una dirección física válida, de lo contrario
     * al retornar de la interrupción se generará un nuevo fallo de página. */

    /* El registro CR2 contiene la dirección virtual que causó el fallo de
     * página. */

    vaddr = read_cr2();

    /* Calcular la página en la cual se encuentra la dirección, eliminando los
     * 12 bits menos significativos de ésta */
    page = vaddr & 0xFFFFF000;
    /* Bloquear el kernel */
    console_printf("Page fault! Unhandled address: 0x%x\n", vaddr);
    for (;;);
}


