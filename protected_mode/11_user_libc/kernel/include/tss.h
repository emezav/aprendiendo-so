/**
 * @file
 * @ingroup kernel_code
 * @brief Definiciones minimas para el Task State Segment del kernel.
 */

#ifndef TSS_H_
#define TSS_H_

#include <pm.h>

/** @brief Task State Segment minimo usado solo para cambios de privilegio. */
typedef struct __attribute__((packed)) tss {
    unsigned short link;
    unsigned short padding_link;
    unsigned int esp0;
    unsigned short ss0;
    unsigned short padding_ss0;
    unsigned int esp1;
    unsigned short ss1;
    unsigned short padding_ss1;
    unsigned int esp2;
    unsigned short ss2;
    unsigned short padding_ss2;
    unsigned int cr3;
    unsigned int eip;
    unsigned int eflags;
    unsigned int eax;
    unsigned int ecx;
    unsigned int edx;
    unsigned int ebx;
    unsigned int esp;
    unsigned int ebp;
    unsigned int esi;
    unsigned int edi;
    unsigned short es;
    unsigned short es_padding;
    unsigned short cs;
    unsigned short cs_padding;
    unsigned short ss;
    unsigned short ss_padding;
    unsigned short ds;
    unsigned short ds_padding;
    unsigned short fs;
    unsigned short fs_padding;
    unsigned short gs;
    unsigned short gs_padding;
    unsigned short ldt_selector;
    unsigned short ldt_selector_padding;
    unsigned short t;
    unsigned short iomap_base;
} tss;

/** @brief TSS con todos sus campos inicializados en cero. */
#define NULL_TSS ((tss){0})

/** @brief Tipo de descriptor TSS disponible de 32 bits. */
#define TASK_TYPE 0x09

/** @brief Instancia minima del TSS del kernel. */
extern tss kernel_tss;

/** @brief Selector del descriptor TSS del kernel dentro de la GDT. */
extern unsigned short kernel_tss_selector;

/**
 * @brief Configura un TSS minimo para soportar futuras entradas desde ring 3.
 */
void setup_tss(void);

#endif /* TSS_H_ */
