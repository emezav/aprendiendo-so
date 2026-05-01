ENTRY(start)

SECTIONS
{
    . = 0x0;

    .text : {
        *(.text*)
        *(.rodata*)
    }

    .data : {
        *(.data*)
        *(.bss*)
    }
}
