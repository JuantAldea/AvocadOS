/* https://interrupt.memfault.com/blog/how-to-write-linker-scripts-for-firmware */

ENTRY(_start_kernel) /* entry point */
OUTPUT_FORMAT(binary)  /* plain binary, no elf, nothing fancy just yet */

SECTIONS
{
    /* 
    By convention, we name those sections as follow:
        .text for code & constants
        .bss for uninitialized data
        .stack for our stack
        .data for initialized data
    */
    . = 1M; /* load 1M into memory. Adress=0x100000 */
    
    /* Reads: <<fill .text section with the .text section of each and every file>>*/
    /* as in  <filename>(section>) */
    .text : ALIGN(4096)
    {
        
        *(.text)
    }

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
