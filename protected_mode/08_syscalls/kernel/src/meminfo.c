/**
 * @file
 * @ingroup kernel_code
 * @brief Comandos de diagnostico de memoria para el monitor.
 */
#include <klog.h>
#include <kmem.h>
#include <meminfo.h>
#include <monitor.h>
#include <paging.h>
#include <physmem.h>
#include <serial.h>

/**
 * @brief Muestra un resumen de memoria fisica y virtual del kernel.
 * @param args Argumentos del comando. No se usan en esta version.
 */
static void meminfo_command_mem(char * args);

/**
 * @brief Ejecuta una prueba de reserva y liberacion de una pagina del kernel.
 * @param args Argumentos del comando. No se usan en esta version.
 */
static void meminfo_command_memtest(char * args);

/**
 * @brief Registra en el monitor los comandos de diagnostico de memoria.
 */
void register_meminfo_monitor_commands(void) {
    monitor_register_command("mem", meminfo_command_mem,
            "show memory summary");
    monitor_register_command("memtest", meminfo_command_memtest,
            "allocate and free one kernel page");
}

/**
 * @brief Callback del comando `mem`.
 * @param args Argumentos crudos del comando. No se usan en esta version.
 */
static void meminfo_command_mem(char * args) {
    unsigned int phys_start;
    unsigned int phys_length;
    unsigned int phys_end;
    unsigned int phys_free_frames;
    unsigned int phys_free_kib;
    unsigned int kmem_start;
    unsigned int kmem_end;
    unsigned int kmem_free_pages;
    unsigned int kmem_free_kib;

    (void)args;

    phys_start = physmem_get_start();
    phys_length = physmem_get_length();
    phys_end = phys_start + phys_length;
    phys_free_frames = available_frames();
    phys_free_kib = (phys_free_frames * FRAME_SIZE) / 1024;

    kmem_start = kmem_get_start();
    kmem_end = kmem_get_end();
    kmem_free_pages = available_pages();
    kmem_free_kib = (kmem_free_pages * PAGE_SIZE) / 1024;

    klog_printf("physmem: start=0x%x end=0x%x size=%u KiB free=%u frames (%u KiB)\n",
            phys_start, phys_end, phys_length / 1024, phys_free_frames,
            phys_free_kib);
    klog_printf("kmem:    start=0x%x end=0x%x free=%u pages (%u KiB)\n",
            kmem_start, kmem_end, kmem_free_pages, kmem_free_kib);
    klog_printf("user:    provisional range=0x%x-0x%x\n",
            USER_VIRT_START, USER_VIRT_END - 1);

    serial_printf("[mem] phys_start=0x%x phys_end=0x%x phys_kib=%u free_frames=%u kmem_start=0x%x kmem_end=0x%x free_pages=%u\n",
            phys_start, phys_end, phys_length / 1024, phys_free_frames,
            kmem_start, kmem_end, kmem_free_pages);
}

/**
 * @brief Callback del comando `memtest`.
 * @param args Argumentos crudos del comando. No se usan en esta version.
 */
static void meminfo_command_memtest(char * args) {
    unsigned int before_frames;
    unsigned int before_pages;
    unsigned int after_alloc_frames;
    unsigned int after_alloc_pages;
    unsigned int after_free_frames;
    unsigned int after_free_pages;
    unsigned int page;

    (void)args;

    before_frames = available_frames();
    before_pages = available_pages();

    page = kmem_allocate_page();
    if (!page) {
        klog_printf("memtest: allocation failed\n");
        serial_printf("[memtest] allocation failed before_frames=%u before_pages=%u\n",
                before_frames, before_pages);
        return;
    }

    after_alloc_frames = available_frames();
    after_alloc_pages = available_pages();

    klog_printf("memtest: allocated page=0x%x\n", page);
    klog_printf("memtest: after alloc frames=%u pages=%u\n",
            after_alloc_frames, after_alloc_pages);
    serial_printf("[memtest] allocated page=0x%x before_frames=%u before_pages=%u after_alloc_frames=%u after_alloc_pages=%u\n",
            page, before_frames, before_pages, after_alloc_frames,
            after_alloc_pages);

    kmem_free(page);

    after_free_frames = available_frames();
    after_free_pages = available_pages();

    klog_printf("memtest: after free frames=%u pages=%u\n",
            after_free_frames, after_free_pages);
    serial_printf("[memtest] after_free_frames=%u after_free_pages=%u\n",
            after_free_frames, after_free_pages);
}
