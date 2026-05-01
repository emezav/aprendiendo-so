/**
 * @file
 * @ingroup kernel_code
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License.
 *
 * @brief Código de inicialización del kernel en C
 *
 * Este codigo recibe el control de start.S y continúa con la ejecución.
*/
#include <asm.h>
#include <stdlib.h>
#include <console.h>

/**
 * @brief Inicializa la consola del kernel y muestra el mensaje de arranque.
 */
void cmain(){
    bochs_break();
    setup_console();
    console_printf("Hello, world!\n");
}
