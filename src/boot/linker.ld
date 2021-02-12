/* https://interrupt.memfault.com/blog/how-to-write-linker-scripts-for-firmware */

ENTRY(_start) /* entry point */
/*OUTPUT_FORMAT(binary)  /* plain binary, no elf, nothing fancy just yet */
OUTPUT_FORMAT(elf32-i386)
SECTIONS
{
    . = 0x07c00;
    .text :
    {
        build/boot/boot.asm.o(.text);
        . = 0x1FE;
        SHORT(0xAA55);
    } = 0xADDEDEC0

    .rodata : ALIGN(4096)
    {
        *(.rodata)
    }

    .data : ALIGN(4096)
    {
        *(.data)
    }

    .bss : ALIGN(4096)
    {
        *(COMMON)
        *(.bss)
    }
    
    .asm : ALIGN(4096)
    {
        *(.asm)
    }
}
