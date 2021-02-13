/* https://interrupt.memfault.com/blog/how-to-write-linker-scripts-for-firmware */

SEARCH_DIR(src)
INCLUDE linker_defines.ld

ENTRY(_start_kernel) /* entry point */
/*OUTPUT_FORMAT(binary)  /* plain binary, no elf, nothing fancy just yet */
OUTPUT_FORMAT(elf32-i386)
SECTIONS
{
    . = __KERNEL_32_LOAD_ADDRESS__;

    start = .;

    .text : {
        build/kernel/kernel.asm.o(.text);
        . = ALIGN(512);
        *(.text)
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
