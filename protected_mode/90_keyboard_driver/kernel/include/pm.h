/**
 * @file
 * @ingroup kernel_code 
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License. 
 * @brief Contiene las definiciones relacionadas con el Modo Protegido IA-32.
 */

#ifndef PM_H_
#define PM_H_

/** @brief Dirección física del kernel en memoria */
#define KERNEL_PHYS_ADDR 0x100000

/** @brief Desplazamiento a partir de la dirección virtual 0x00000000 al cual se
 * mapea el kernel en la memoria virtual */
#define KERNEL_VIRT_OFFSET 0xC0000000

/** @brief Dirección virtual del kernel en memoria */
#define KERNEL_VIRT_ADDR KERNEL_PHYS_ADDR + KERNEL_VIRT_OFFSET

/* @brief Tamaño de la pila del kernel */ 
#define KERNEL_STACK_SIZE 0x4000

/* @brief Final de la memoria disponible por debajo del primer MB de memoria */
#define LOWMEM_STACK_ADDR 0x9FC00

/** @brief Número máximo de entradas de la GDT */
#define MAX_GDT_ENTRIES 16

/** @brief Tipo de segmento de código */
#define CODE_SEGMENT 0x9A
/** @brief Tipo de segmento de datos*/
#define DATA_SEGMENT 0x92

/** @brief Desplazamiento en bytes dentro de la GDT a partir del cual se
 * encuentra el descriptor de segmento de código del kernel. Se debe tener en
 * cuenta que cada descriptor de segmento ocupa 8 bytes. */
#define KERNEL_CODE_SELECTOR 0x08

/** @brief Desplazamiento en bytes dentro de la GDT a partir del cual se
 * encuentra el descriptor de segmento de datos del kernel. Se debe tener en
 * cuenta que cada descriptor de segmento ocupa 8 bytes. */
#define KERNEL_DATA_SELECTOR 0x10

/** @brief Nivel de privilegios 0*/
#define RING0_DPL 0
/** @brief Nivel de privilegios 1*/
#define RING1_DPL 1
/** @brief Nivel de privilegios 2*/
#define RING2_DPL 2
/** @brief Nivel de privilegios 3*/
#define RING3_DPL 3

/* Constantes relacionadas con las interrupciones */

/** @brief Número de entradas en la IDT: 256 en la arquitectura IA-32. */
#define MAX_IDT_ENTRIES 256

/** @brief Constante para el tipo de descriptor 'interrupt_gate' */
#define INTERRUPT_GATE_TYPE 0x0E

/** @brief Valor de el registro EFLAGS, con el bit IF = 1. El bit 1 siempre debe
 * ser 1. */
#define IF_ENABLE 0x202

/* Dado que este archivo puede ser incluido desde codigo en Assembler, incluir
 * solo las constantes definidas anteriormente. */

#ifndef ASM

/** @brief Tipo para un descriptor de segmento */
typedef struct __attribute__((packed)) {
	/* Bits menos significativos del descriptor. Agrupan
	* Limite 0..15 y Base 0..15 del descriptor */
	unsigned int low : 32;
	/* Bits más significativos del descriptor. Agrupan Base 16..23, Tipo,
	* S, DPL, P, Límite, AVL, L, D/B, G y Base 24..31 */
	unsigned int high: 32;
}gdt_descriptor;

/** @brief Estructura de datos para un descriptor de segmento
 * @details
De acuerdo con el manual de Intel, un descriptor de segmento es una estructura
de datos que ocupa 8 bytes (64 bits), con el siguiente formato (Ver Cap. 3,
Seccion 3.4.5, en Intel 64 and IA-32 Architectures Software Developer’s Manual
Volume 3A: System Programming Guide, Part 1)

@verbatim
31                               16     12 11    8 7              0
 ------------------------------------------------------------------
|                | |D| |A|        | |   | |       |                |
|  Base 24..31   |G|/|L|V| Limite |P|DPL|S| Tipo  |   Base 16..23  |
|                | |B| |L| 16..19 | |   | |       |                |
 ------------------------------------------------------------------  MSB
 31                               16 15                           0
 ------------------------------------------------------------------
|                                 |                                |
|        Base 0..15               |          Limite 0..15          |
|                                 |                                |
 ------------------------------------------------------------------  LSB

@endverbatim

La distribución de los bits dentro del descriptor se explica a continuación:
- Los bits Base 0..31 permiten definir la base del segmento en el espacio
lineal de 32 bits.
- Los bits Limite 0..19 permiten definir el tamaño del segmento. Este límite
también se relaciona con el bit G (Granularidad) de la siguiente forma:
   - Si G es 0, el tamaño del segmento en bytes es igual al valor
     almacenado en Limite
   - Si G = 1, el tamaño del segmento es el valor almacenado en Limite
     multiplicado por 4096.
- El bit D/B funciona diferente para para segmentos de codigo y datos. Para modo
protegido de 32 bits este bit debe tener valor de 1. Consulte el manual
de Intel para más detalles.
- El bit L debe ser 0 para modo protegido de 32 bits.
- El bit AVL puede ser tener como valor 1 o 0. Por defecto se toma 0.
- El bit P es 1 si el segmento está presente, 0 en caso contrario.
- El bit DPL define el nivel de privilegios del descriptor. Por tener 2 bits
  puede almacenar tres valores: 0, 1 y 2. 0 es el mayor privilegio.
- El bit S para descriptores de segmento de codigo o datos debe ser 1.
- Los bits correspondientes a Tipo definen el tipo de segmento. Vea la sección
3.5.1 del manual de Intel mencionado.
   - Para segmentos de código, Tipo tiene el valor binario de 1010 = 0xA
   - Para segmentos de datos, Tipo tiene el valor binario de 0010 = 0x2
   Si se combinan los bits P=1, DPL=0 y S=1 junto con el valor de Tipo, se
   obtiene un byte con valor 0x9A para un descriptor de segmento de código y un
   valor de 0x92 para un descriptor de segmento de datos.
*/

/** @brief Definición de la estructura de datos para un descriptor de
 * interrupción */
typedef struct __attribute((packed)) {
	/** Bits menos significativos del desplazamiento dentro del segmento de
	 * código en el cual se encuentra la rutina de manejo de interrupción */
	unsigned short offset_low  : 16;
	/** Selector del segmento de código en el cual se encuentra la rutina de
	 * manejo de interrupción */
	unsigned short selector : 16;
	/** Tipo del descriptor */
	unsigned short type : 16;
	/** Bits más significativos del desplazamiento dentro del segmento de
	 * código en el cual se encuentra la rutina de manejo de interrupción */
	unsigned short offset_high: 16;
}idt_descriptor;

/** @brief Tabla Global de Descriptores (GDT). Es un arreglo de descriptores
 * de segmento. Según el manual de Intel, esta tabla debe estar alineada a un
 * límite de 8 bytes para un óptimo desempeño. */
extern gdt_descriptor kernel_gdt[];

/** @brief Referencia a la tabla de rutinas de servicio de interrupcion.
 *  Esta tabla se encuentra definida en el archivo start.S. */
extern unsigned int isr_table[];

/** @brief Referencia a la tabla de descriptores de interrupcion */
extern idt_descriptor kernel_idt[];

extern unsigned int kernel_idt_pointer;

extern unsigned int current_esp;

extern unsigned int kernel_stack_end;

/**
 * @brief Función que permite obtener el selector en la GDT a partir de un
 * apuntador a un descriptor de segmento
 * @param desc Referencia al descriptor de segmento del cual se desea obtener
 * el selector
 * @return Selector que referencia al descriptor dentro de la GDT.
 */
unsigned short get_gdt_selector(gdt_descriptor * desc);

/**
 * @brief Función que permite obtener el descriptor de segmento en la GDT a
 * partir de un selector
 * @param selector Selector que permite obtener un descriptor de segmento
 * de la GDT
 * @return Referencia al descriptor de segmento dentro de la GDT
 */
gdt_descriptor * get_gdt_descriptor(unsigned short selector);

/**
 * @brief Permite obtener un descriptor de segmento
 *  disponible en la GDT.
 */
gdt_descriptor * allocate_gdt_descriptor(void);

/**
 * @brief Permite obtener un selector de un descriptor de segmento
 *  disponible en la GDT.
 *  @return Referencia al próximo descriptor de segmento dentro de la GDT que se
 *  encuentra disponible, nulo en caso que no exista una entrada disponible
 *  dentro de la GDT.
 */
unsigned short allocate_gdt_selector(void);

/**
 * @brief Permite liberar un descriptor de segmento en la GDT.
 * @param desc Apuntador al descriptor que se desa liberar
 * */
void free_gdt_descriptor(gdt_descriptor *);

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
void setup_gdt_descriptor(unsigned short , unsigned int,
		unsigned int, char, char, int, char);

/** @brief Estructura que define el estado del procesador al recibir una
 * interrupción o una excepción.
 * @details Al recibir una interrupción, el procesador automáticamente almacena
 * en la pila el valor de CS, EIP y EFLAGS. Si la interrupción ocurrió en un
 * nivel de privilegios diferente de cero, antes de almacenar CS, EIP y EFLAGS
 * se almacena el valor de SS y ESP.
 * El control lo recibe el código del archivo start.S, en el cual almacena
 * (en orden inverso) el estado del procesador contenido en esta estructura.
 */
typedef struct interrupt_state {
	/** @brief Valor del selector GS (Tope de la pila) */
	unsigned int gs;
	/** @brief Valor del selector FS */
	unsigned int fs;
	/** @brief Valor del selector ES */
	unsigned int es;
	/** @brief Valor del selector DS */
	unsigned int ds;
	/** @brief Valor del registro EDI */
	unsigned int edi;
	/** @brief Valor del registro  ESI */
	unsigned int esi;
	/** @brief Valor del registro  EBP */
	unsigned int ebp;
	/** @brief Valor del registro  ESP */
	unsigned int esp;
	/** @brief Valor del registro  EBX */
	unsigned int ebx;
	/** @brief Valor del registro  EDX */
	unsigned int edx;
	/** @brief Valor del registro  ECX */
	unsigned int ecx;
	/** @brief Valor del registro  EAX */
	unsigned int eax;
	/** @brief Número de la interrupción (o excepción) */
	unsigned int number;
	/** @brief Código de error. Cero para las excepciones que no generan
	 * código de error y para las interrupciones. */
	unsigned int error_code;
	/** @brief Valor de EIP en el momento en que ocurrió la interrupción
	 * (almacenado automáticamente por el procesador) */
	unsigned int old_eip;
	/** @brief Valor de CS en el momento en que ocurrió la interrupción
	* (almacenado automáticamente por el procesador) */
	unsigned int old_cs;
	/** @brief Valor de EFLAGS en el momento en que ocurrió la interrupción
	* (almacenado automáticamente por el procesador) */
	unsigned int old_eflags;
	/** @brief Valor de ESP en el momento en que ocurrió la interrupción
	* (almacenado automáticamente por el procesador). Sólo se almacena cuando
	* la interrupción o excepción ocurrió cuado una tarea de privilegio
	* mayor a cero se estaba ejecutando. */
	unsigned int old_esp;
	/** @brief Valor de SS en el momento en que ocurrió la interrupción
	* (almacenado automáticamente por el procesador). Sólo se almacena cuando
	* la interrupción o excepción ocurrió cuado una tarea de privilegio
	* mayor a cero se estaba ejecutando. */
	unsigned int old_ss;
} interrupt_state;


/** @brief Definición de tipo para las rutinas de manejo de interrupcion */
typedef void (*interrupt_handler)(interrupt_state *);

/** @brief Constante para definir un manejador de interrupcion vacio*/
#define NULL_INTERRUPT_HANDLER (interrupt_handler)0
/**
 * @brief Configura y carga la IDT que se usa en modo protegido.
 */
void setup_interrupts(void);

/**
 * @brief Instala un nuevo manejador de interrupción para un número de
 * interrupción determinado.
 * @param index Número de interrupción para la cual se desea instalar el
 * manejador
 * @param handler Función para el manejo de la interrupción.
 */
void install_interrupt_handler(unsigned char index, interrupt_handler handler);

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
		unsigned int offset, unsigned char dpl, unsigned char type);

/**
 * @brief Muestra el estado en el cual
 * se encuentra el procesador cuando ocurrio una interrupción.
 * @param state Apuntador al marco de pila de interrupción.
 */
void dump_interrupt_state(interrupt_state * state);
#endif

#endif /* PM_H_ */
