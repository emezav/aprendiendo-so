ENTRY(start)

SECTIONS
{
    . = 0x410000;

    .text : {
        *(.text*)
        *(.rodata*)
    }

    .data : {
        *(.data*)
        *(.bss*)
    }
}
