#include <asm.h>
#include <keyboard.h>
#include <string.h>

/* */
char keyboard_buf[BUFSIZ];
int keyboard_pos;

char es_kb[KB_KEYS];
char es_kb2[KB_KEYS];
char es_kb3[KB_KEYS];

kb_mod_keys mod_keys;

void setup_keyboard(void) {
    int i;

    for (i = 0; i < 128; i++) {
        es_kb[i] = 0;
        es_kb2[i] = 0;
        es_kb3[i] = 0;
    }


    /* Configurar el mapa de teclado */
    memset(&mod_keys, 0, sizeof(kb_mod_keys));
    memset(&es_kb, 0, sizeof(es_kb));
    memset(&es_kb2, 0, sizeof(es_kb2));
    memset(&es_kb3, 0, sizeof(es_kb3));

    es_kb[1]  = 27;      es_kb2[1]  = 27; //ESC
    es_kb[2]  = '1';     es_kb2[2]  = '!';
    es_kb[3]  = '2';     es_kb2[3]  = '"';
    es_kb[4]  = '3';     es_kb2[4]  = '#';
    es_kb[5]  = '4';     es_kb2[5]  = '$';
    es_kb[6]  = '5';     es_kb2[6]  = '%';
    es_kb[7]  = '6';     es_kb2[7]  = '&';
    es_kb[8]  = '7';     es_kb2[8]  = '/';
    es_kb[9]  = '8';     es_kb2[9]  = '(';
    es_kb[10] = '9';     es_kb2[10]  = ')';
    es_kb[11] = '0';     es_kb2[11]  = '=';
    es_kb[12] = '-';     es_kb2[12]  = '_';
    es_kb[13] = '=';     es_kb2[13]  = '+';
    es_kb[14] = '\b';    es_kb2[14]  = '\b';
    es_kb[15] = '\t';    es_kb2[15]  = '\t';
    es_kb[16] = 'q';     es_kb2[16]  = 'Q';    es_kb3[16] = '@';
    es_kb[17] = 'w';     es_kb2[17]  = 'W';
    es_kb[18] = 'e';     es_kb2[18]  = 'E';
    es_kb[19] = 'r';     es_kb2[19]  = 'R';
    es_kb[20] = 't';     es_kb2[20]  = 'T';
    es_kb[21] = 'y';     es_kb2[21]  = 'Y';
    es_kb[22] = 'u';     es_kb2[22]  = 'U';
    es_kb[23] = 'i';     es_kb2[23]  = 'I';
    es_kb[24] = 'o';     es_kb2[24]  = 'O';
    es_kb[25] = 'p';     es_kb2[25]  = 'P';
    es_kb[26] = '[';     es_kb2[26]  = ']';
    es_kb[27] = '+';     es_kb2[27]  = '*';
    es_kb[28] = '\n';    es_kb2[28]  = '\n';

    es_kb[30] = 'a';     es_kb2[30]  = 'A';
    es_kb[31] = 's';     es_kb2[31]  = 'S';
    es_kb[32] = 'd';     es_kb2[32]  = 'D';
    es_kb[33] = 'f';     es_kb2[33]  = 'F';
    es_kb[34] = 'g';     es_kb2[34]  = 'G';
    es_kb[35] = 'h';     es_kb2[35]  = 'H';
    es_kb[36] = 'j';     es_kb2[36]  = 'J';
    es_kb[37] = 'k';     es_kb2[37]  = 'K';
    es_kb[38] = 'l';     es_kb2[38]  = 'L';
    es_kb[39] = 'n';     es_kb2[39]  = 'N';
    es_kb[40] = '{';     es_kb2[40]  = '[';

    es_kb[41] = '\\';    es_kb2[41]  = '|';

    es_kb[43] = '}';     es_kb2[43]  = ']';

    es_kb[44] = 'z';     es_kb2[44]  = 'Z';
    es_kb[45] = 'x';     es_kb2[45]  = 'X';
    es_kb[46] = 'c';     es_kb2[46]  = 'C';
    es_kb[47] = 'v';     es_kb2[47]  = 'V';
    es_kb[48] = 'b';     es_kb2[48]  = 'B';
    es_kb[49] = 'n';     es_kb2[49]  = 'N';
    es_kb[50] = 'm';     es_kb2[50]  = 'M';
    es_kb[51] = ',';     es_kb2[51]  = ';';
    es_kb[52] = '.';     es_kb2[52]  = ':';
    es_kb[53] = '-';     es_kb2[53]  = '_';

    es_kb[57] = ' ';     es_kb2[57]  = ' ';

    /* Instalar el manejador de la IRQ del teclado (IRQ1)*/
    install_irq_handler(KB_IRQ, keyboard_handler);

}

void keyboard_handler(interrupt_state * state) {
    unsigned char code;
    unsigned char status;
    unsigned char c;

    do {
        status = inb(KB_STATUS_PORT);
    }while (!(status & 1));

    code = inb(KB_OUTPUT_PORT);

    //Teclas de control
    switch (code) {
        case 0x2a:  //Left shift, make
        case 0x36:  //Right shift, make
            mod_keys.shift = 1;
            break;
        case 0xaa:  //Left shift, break
        case 0xb6:  //Right shift, break
            mod_keys.shift = 0;
            break;
        case 0x38:
            mod_keys.alt = 1; //Left alt, make
            break;
        case 0xb8:
            mod_keys.alt = 0; //Left alt, break
            break;
        case 0x1d:
            mod_keys.ctrl = 1; //Ctrl, make
            break;
        case 0x9d:
            mod_keys.ctrl = 0; //Ctrl, break
            break;
        case 0x3A:
            mod_keys.shiftlock = 1 - mod_keys.shiftlock; //Shift lock
            break;
    }

   /* 
    console_printf("%d:%d:%d:%d [0x%x %d]\n", 
            mod_keys.shift,
            mod_keys.ctrl,
            mod_keys.alt,
            mod_keys.shiftlock,
            code,
            (char)code & 0x7F);
   */

    //TODO traducir el caracter ASCII correspondiente y colocar en el
    if (code & KB_BREAK) { 
        code -= KB_BREAK; //Remove break code
        if (code < KB_KEYS) {
            c = es_kb[code];
            if (mod_keys.shiftlock && 
                !mod_keys.shift && 
                    code >= 0x10 &&  // Q
                    code <= 0x32) {  // M
                c = es_kb2[code];
            }
            if (mod_keys.shift &&
                    !mod_keys.shiftlock) {
                c = es_kb2[code];
            }
            if (mod_keys.shift && 
                    mod_keys.shiftlock && 
                    code > 0x01 && 
                    code <= 0x35) {
                c = es_kb2[code];
            }
            if (mod_keys.ctrl &&
                    mod_keys.alt
                    && es_kb3[code] != 0) {
                c = es_kb3[code];
            }
            if (c) {
                console_printf("%c", c);
            }
        }
    }
}


char getchar() {
    //TODO ! saca un caracter del buffer!
}

char * gets(int sz) {
    //TODO saca sz caracteres del buffer!
}


