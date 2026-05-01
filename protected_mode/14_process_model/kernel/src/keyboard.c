/**
 * @file
 * @ingroup kernel_code
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License.
 *
 * @brief Implementacion basica del controlador de teclado PS/2.
 */

#include <asm.h>
#include <irq.h>
#include <pm.h>
#include <serial.h>
#include <keyboard.h>

/** @brief Ultimo scancode recibido. */
static unsigned char keyboard_last_scancode = 0;

/** @brief Ultimo caracter ASCII traducido. */
static char keyboard_last_ascii = 0;

/** @brief Estado combinado de las teclas Shift. */
static int keyboard_shift_active = 0;

/** @brief Estado de Caps Lock. */
static int keyboard_caps_lock = 0;

/** @brief Buffer circular de caracteres del teclado. */
static char keyboard_buffer[KEYBOARD_BUFFER_SIZE];

/** @brief Posicion de escritura dentro del buffer circular. */
static unsigned int keyboard_buffer_head = 0;

/** @brief Posicion de lectura dentro del buffer circular. */
static unsigned int keyboard_buffer_tail = 0;

/** @brief Cantidad de caracteres almacenados en el buffer. */
static unsigned int keyboard_buffer_count = 0;

/**
 * @brief Traduce un scancode set 1 a ASCII para un subconjunto basico.
 * @param scancode Scancode a traducir.
 * @return Caracter ASCII correspondiente, o cero si no existe traduccion.
 */
static char keyboard_scancode_to_ascii(unsigned char scancode);
static int keyboard_is_alpha(char c);

/**
 * @brief Inserta un caracter en el buffer circular del teclado.
 * @param c Caracter a almacenar.
 */
static void keyboard_buffer_put(char c);

/**
 * @brief Manejador privado de la IRQ1 del teclado.
 * @param state Marco de pila de la interrupcion.
 */
static void keyboard_handler(interrupt_state * state);

void setup_keyboard(void) {
    keyboard_last_scancode = 0;
    keyboard_last_ascii = 0;
    keyboard_shift_active = 0;
    keyboard_caps_lock = 0;
    keyboard_buffer_head = 0;
    keyboard_buffer_tail = 0;
    keyboard_buffer_count = 0;
    install_irq_handler(1, keyboard_handler);
}

unsigned char keyboard_get_last_scancode(void) {
    return keyboard_last_scancode;
}

char keyboard_get_last_ascii(void) {
    return keyboard_last_ascii;
}

int keyboard_has_char(void) {
    return keyboard_buffer_count > 0;
}

char keyboard_getchar(void) {
    char c;

    if (keyboard_buffer_count == 0) {
        return 0;
    }

    c = keyboard_buffer[keyboard_buffer_tail];
    keyboard_buffer_tail = (keyboard_buffer_tail + 1) % KEYBOARD_BUFFER_SIZE;
    keyboard_buffer_count--;
    return c;
}

static char keyboard_scancode_to_ascii(unsigned char scancode) {
    char ascii;

    switch (scancode) {
        case 0x02: ascii = keyboard_shift_active ? '!' : '1'; break;
        case 0x03: ascii = keyboard_shift_active ? '@' : '2'; break;
        case 0x04: ascii = keyboard_shift_active ? '#' : '3'; break;
        case 0x05: ascii = keyboard_shift_active ? '$' : '4'; break;
        case 0x06: ascii = keyboard_shift_active ? '%' : '5'; break;
        case 0x07: ascii = keyboard_shift_active ? '^' : '6'; break;
        case 0x08: ascii = keyboard_shift_active ? '&' : '7'; break;
        case 0x09: ascii = keyboard_shift_active ? '*' : '8'; break;
        case 0x0A: ascii = keyboard_shift_active ? '(' : '9'; break;
        case 0x0B: ascii = keyboard_shift_active ? ')' : '0'; break;
        case 0x0C: ascii = keyboard_shift_active ? '_' : '-'; break;
        case 0x0D: ascii = keyboard_shift_active ? '+' : '='; break;
        case 0x0E: ascii = '\b'; break;
        case 0x0F: ascii = '\t'; break;
        case 0x10: ascii = 'q'; break;
        case 0x11: ascii = 'w'; break;
        case 0x12: ascii = 'e'; break;
        case 0x13: ascii = 'r'; break;
        case 0x14: ascii = 't'; break;
        case 0x15: ascii = 'y'; break;
        case 0x16: ascii = 'u'; break;
        case 0x17: ascii = 'i'; break;
        case 0x18: ascii = 'o'; break;
        case 0x19: ascii = 'p'; break;
        case 0x1A: ascii = keyboard_shift_active ? '{' : '['; break;
        case 0x1B: ascii = keyboard_shift_active ? '}' : ']'; break;
        case 0x1C: ascii = '\n'; break;
        case 0x1E: ascii = 'a'; break;
        case 0x1F: ascii = 's'; break;
        case 0x20: ascii = 'd'; break;
        case 0x21: ascii = 'f'; break;
        case 0x22: ascii = 'g'; break;
        case 0x23: ascii = 'h'; break;
        case 0x24: ascii = 'j'; break;
        case 0x25: ascii = 'k'; break;
        case 0x26: ascii = 'l'; break;
        case 0x27: ascii = keyboard_shift_active ? ':' : ';'; break;
        case 0x28: ascii = keyboard_shift_active ? '"' : '\''; break;
        case 0x29: ascii = keyboard_shift_active ? '~' : '`'; break;
        case 0x2B: ascii = keyboard_shift_active ? '|' : '\\'; break;
        case 0x2C: ascii = 'z'; break;
        case 0x2D: ascii = 'x'; break;
        case 0x2E: ascii = 'c'; break;
        case 0x2F: ascii = 'v'; break;
        case 0x30: ascii = 'b'; break;
        case 0x31: ascii = 'n'; break;
        case 0x32: ascii = 'm'; break;
        case 0x33: ascii = keyboard_shift_active ? '<' : ','; break;
        case 0x34: ascii = keyboard_shift_active ? '>' : '.'; break;
        case 0x35: ascii = keyboard_shift_active ? '?' : '/'; break;
        case 0x39: ascii = ' '; break;
        default: return 0;
    }

    if (keyboard_is_alpha(ascii)) {
        if ((keyboard_shift_active && !keyboard_caps_lock)
                || (!keyboard_shift_active && keyboard_caps_lock)) {
            ascii = (char)(ascii - ('a' - 'A'));
        }
    }

    return ascii;
}

static int keyboard_is_alpha(char c) {
    return c >= 'a' && c <= 'z';
}

static void keyboard_buffer_put(char c) {
    if (keyboard_buffer_count == KEYBOARD_BUFFER_SIZE) {
        return;
    }

    keyboard_buffer[keyboard_buffer_head] = c;
    keyboard_buffer_head = (keyboard_buffer_head + 1) % KEYBOARD_BUFFER_SIZE;
    keyboard_buffer_count++;
}

static void keyboard_handler(interrupt_state * state) {
    unsigned char scancode;
    char ascii;

    (void) state;

    if ((inb(KEYBOARD_STATUS_PORT) & KEYBOARD_OUTPUT_BUFFER_FULL) == 0) {
        return;
    }

    scancode = inb(KEYBOARD_DATA_PORT);
    keyboard_last_scancode = scancode;

    if (scancode == 0x2A || scancode == 0x36) {
        keyboard_shift_active = 1;
        keyboard_last_ascii = 0;
        serial_printf("[keyboard] scancode=0x%x shift=down caps=%u\n",
                scancode,
                keyboard_caps_lock);
        return;
    }

    if (scancode == 0xAA || scancode == 0xB6) {
        keyboard_shift_active = 0;
        keyboard_last_ascii = 0;
        serial_printf("[keyboard] scancode=0x%x shift=up caps=%u\n",
                scancode,
                keyboard_caps_lock);
        return;
    }

    if (scancode == 0x3A) {
        keyboard_caps_lock = !keyboard_caps_lock;
        keyboard_last_ascii = 0;
        serial_printf("[keyboard] scancode=0x%x caps=%u\n",
                scancode,
                keyboard_caps_lock);
        return;
    }

    if (scancode & 0x80) {
        keyboard_last_ascii = 0;
        return;
    }

    ascii = keyboard_scancode_to_ascii(scancode);
    keyboard_last_ascii = ascii;

    if (ascii != 0) {
        keyboard_buffer_put(ascii);
    }

    if (ascii >= ' ' && ascii <= '~') {
        serial_printf("[keyboard] scancode=0x%x ascii=%c\n", scancode, ascii);
    } else if (ascii == '\n') {
        serial_printf("[keyboard] scancode=0x%x ascii=ENTER\n", scancode);
    } else if (ascii == '\b') {
        serial_printf("[keyboard] scancode=0x%x ascii=BACKSPACE\n", scancode);
    } else if (ascii == '\t') {
        serial_printf("[keyboard] scancode=0x%x ascii=TAB\n", scancode);
    } else {
        serial_printf("[keyboard] scancode=0x%x\n", scancode);
    }
}
