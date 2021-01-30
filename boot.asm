; http://www.ctyme.com/rbrown.htm
ORG 0x7c00
BITS 16 ; 16 bit code (for the assembler)

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; account for the BIOS Parameter block BPB
; https://wiki.osdev.org/FAT#BPB_.28BIOS_Parameter_Block.29
_start:
    jmp short init_code_segment
    nop
; filler, as the BIOS might write the BPB into this section.
times 33 db 0

; GDT
gdt_start:
gdt_null:
    dd 0x0
    dd 0x0

; offset 0x8
gdt_code:       ; CS should point to this
    dw 0xffff   ; segment limit 0-15
    dw 0x0      ; base 0-15 bits
    db 0x0      ; base 16-23
    db 0x9a     ; access byte
    db 11001111b ; high & low 4 bit  fags
    db 0x0       ; base 24-31

;offset 0x10
gdt_data:       ; ds, ss, es, fs, gs
    dw 0xffff   ; segment limit 0-15
    dw 0x0      ; base 0-15 bits
    db 0x0      ; base 16-23
    db 0x92     ; access byte
    db 11001111b ; high & low 4 bit  fags
    db 0x0       ; base 24-31

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; length of the descriptor
    dd gdt_start    ; address of the descriptor

init_code_segment:
    ; The (legacy) BIOS checks bootable devices for a boot signature, a so called magic number
    ; The boot signature is in a boot sector (sector number 0) and it contains the byte sequence
    ; 0x55, 0xAA at byte offsets 510 and 511 respectively. When the BIOS finds such a boot sector,
    ; it is loaded into memory at 0x0000:0x7c00 (segment 0, address 0x7c00). 
    ; However, some BIOS' load to 0x7c0:0x0000 (segment 0x07c0, offset 0), which resolves to the
    ; same physical address, but can be surprising. 
    ; A good practice is to enforce CS:IP at the very start of your boot sector.
    jmp 0:start ; now the code is located at 0x7c00 so segment is 0x7c00

start:
    cli ; int off TODO: should this be performed here or before?
    mov ax, 0x00 ; segment offset is 0
    mov ds, ax ; data segment
    mov es, ax ; special segment
    mov ss, ax ; stack segment
    mov sp, 0x7c00 ; stack pointer
    sti ; int on

.load_protected:
    cli
    lgdt[gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:load32

[BITS 32]
load32:
    ; now the BIOS is out of reach
    ; set data registers
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ; set stack pointer
    mov ebp, 0x00200000
    mov esp, ebp

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

    jmp $

times 510 - ($ - $$) db 0 ; fill 510 bytes as 0 (510 - (current_addr - begin)))

dw 0xAA55; (little endian, 0x055AA is the boot signature)

; BIOS wont load anthing from here, as it loads only 512 bytes
