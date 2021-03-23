/* linker script for programs */
ENTRY(_program_start)

OUTPUT_FORMAT(binary)

SECTIONS
{
    . = 0x400000;
    .text :
    {
        *(.text)
    }

    .rodata : ALIGN(4K)
    {
        *(.rodata)
    }

    .data : ALIGN(4K)
    {
        *(.data)
    }

    .bss : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }

    .asm : ALIGN(4K)
    {
        *(.asm)
    }
}
