/**
 * @file
 * @ingroup kernel_code
 * @brief Implementacion del TSS minimo del kernel para futuras entradas desde userland.
 */

#include <asm.h>
#include <pm.h>
#include <tss.h>

/** @brief Instancia del TSS minimo del kernel. */
tss kernel_tss __attribute__((aligned(8)));

/** @brief Selector del descriptor TSS del kernel dentro de la GDT. */
unsigned short kernel_tss_selector;

/**
 * @brief Configura un TSS minimo para soportar futuras entradas desde ring 3.
 */
void setup_tss(void) {
    kernel_tss = NULL_TSS;
    kernel_tss.ss0 = kernel_data_selector;
    kernel_tss.esp0 = (unsigned int)&kernel_stack_end;
    kernel_tss.iomap_base = sizeof(tss);

    if (kernel_tss_selector == 0) {
        kernel_tss_selector = allocate_gdt_selector();
    }

    if (kernel_tss_selector == 0) {
        return;
    }

    setup_gdt_descriptor(kernel_tss_selector, (unsigned int)&kernel_tss,
            sizeof(tss) - 1, TASK_TYPE, RING0_DPL, 0, 0);
    inline_assembly("ltr %0"::"g"(kernel_tss_selector));
}
