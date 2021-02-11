; http://www.ctyme.com/rbrown.htm
; ORG 0x7c00 ; commented out because we are generating an ELF file to get debug symbols. See Makefile
section .text
[BITS 16] ; 16 bit code (for the assembler)

[global _start]

;extern KERNEL_32_BUFFER
KERNEL_32_BUFFER equ 0x0100000 ; 1MB
; account for the BIOS Parameter block BPB
; https://wiki.osdev.org/FAT#BPB_.28BIOS_Parameter_Block.29
_start:
    jmp short init_code_segment
    nop

times 33 db 0 ; filler, as the BIOS might write its BPB into this section.

%include "gdt.inc"

idt_descriptor:
    dw 0x0
    dd 0x0

init_code_segment:
    cli ; int off
    ; The (legacy) BIOS checks bootable devices for a boot signature, a so called magic number
    ; The boot signature is in a boot sector (sector number 0) and it contains the byte sequence
    ; 0x55, 0xAA at byte offsets 510 and 511 respectively. When the BIOS finds such a boot sector,
    ; it is loaded into memory at 0x0000:0x7c00 (segment 0, address 0x7c00).
    ; However, some BIOS' load to 0x7c0:0x0000 (segment 0x07c0, offset 0), which resolves to the
    ; same physical address, but can be surprising.
    ; A good practice is to enforce CS:IP at the very start of your boot sector.
    jmp dword 0:set_segments
set_segments:
    xor ax, ax ; segment offset is 0
    mov ds, ax ; data segment
    mov es, ax ; special segment
    mov ss, ax ; stack segment
    mov sp, 0x2000 ; stack pointer

.load_protected:
    ; enable A20 line (21th bit)
    ; assumming fast A20 latch
    ; However, the Fast A20 method is not supported everywhere and there is no reliable way
    ; to tell if it will have some effect or not on a given system. Even worse, on some systems,
    ; it may actually do something else like blanking the screen, so it should be used only after
    ; the BIOS has reported that FAST A20 is available. Code for systems lacking FAST A20 support
    ; is also needed, so relying only on this method is discouraged. Also, on some chipsets you
    ; might have to enable Fast A20 support in the BIOS configuration screen.
    in al, 0x92
    or al, 2
    out 0x92, al

    ; load GDT table
    lgdt[dword gdt_descriptor]
    lidt[dword idt_descriptor]

    ; load status from control register 0, set bit 0 and put it back
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    mov ax, DATA_SEG
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

    ; protected mode will activate when CS is updated -> perform a long jump
    jmp dword CODE_SEG:load32

[BITS 32]
load32:
    ; load the early kernel code from disk and jump to it

    mov eax, 1 ; sector 0 is the boot sector, we want to load from sector 1
    mov ecx, 100 ; 100 sectors
    mov edi, KERNEL_32_BUFFER
    call ata_lba_read
    jmp CODE_SEG:KERNEL_32_BUFFER

%include "ata_lba_read.inc"
;times 510 - ($ - $$) db 0 ; fill 510 bytes as 0 (510 - (current_addr - begin)))
; 510 instead of 512 because the last two store the boot signature
;dw 0xAA55; (little endian, 0x55AA is the boot signature)

; BIOS wont load anthing from here, as it only loads 512 bytes
