/* https://interrupt.memfault.com/blog/how-to-write-linker-scripts-for-firmware */

SEARCH_DIR(src)
INCLUDE linker_defines.ld

/* entry point */
ENTRY(_start_kernel)

/* OUTPUT_FORMAT(binary) */

OUTPUT_FORMAT(elf32-i386)

ENTRY(kernel_trampoline_low)

SECTIONS
{
    . = __KERNEL_32_LOAD_ADDRESS__;

    /* can be used to determine kernel's size */


    /*
        kernel will be stored from __KERNEL_32_LOAD_ADDRESS__,
        but it will be vm-mapped from __KERNEL_VIRTUAL_ADDRESS_BASE__
        -> it should believe it resides there
    */

    . += __KERNEL_VIRTUAL_ADDRESS_BASE__;
    _kernel_start = .;
    .text : AT (ADDR (.text) - __KERNEL_VIRTUAL_ADDRESS_BASE__)
    {
        build/kernel/kernel.asm.o(.text);
        . = ALIGN(512);
        *(.text)
    } = 0xADDEDEC0

    .rodata ALIGN(4K) : AT (ADDR (.rodata) - __KERNEL_VIRTUAL_ADDRESS_BASE__)
    {
        *(.rodata)
    }

    .data ALIGN(4K) : AT (ADDR (.data) - __KERNEL_VIRTUAL_ADDRESS_BASE__)
    {
        *(.data)
    }

    .bss ALIGN(4K) : AT (ADDR (.bss) - __KERNEL_VIRTUAL_ADDRESS_BASE__)
    {
        *(COMMON)
        *(.bss)
    }

    . = ALIGN(4K);

    _kernel_end = .;
}
