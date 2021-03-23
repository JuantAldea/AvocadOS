/* https://interrupt.memfault.com/blog/how-to-write-linker-scripts-for-firmware */

SEARCH_DIR(src)
INCLUDE linker_defines.ld

ENTRY(_fat_header_start) /* entry point, only used by elf loaders */
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
}
