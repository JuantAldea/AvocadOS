; http://www.ctyme.com/rbrown.htm
ORG 0
BITS 16 ;16 bit code (for the assembler)
; account for the BIOS Parameter block BPB
; https://wiki.osdev.org/FAT#BPB_.28BIOS_Parameter_Block.29
_start:
    jmp short init_code_segment
    nop
; filler, as the BIOS might write the BPB into this section.
times 33 db 0

init_code_segment:
    ; The (legacy) BIOS checks bootable devices for a boot signature, a so called magic number
    ; The boot signature is in a boot sector (sector number 0) and it contains the byte sequence
    ; 0x55, 0xAA at byte offsets 510 and 511 respectively. When the BIOS finds such a boot sector,
    ; it is loaded into memory at 0x0000:0x7c00 (segment 0, address 0x7c00). 
    ; However, some BIOS' load to 0x7c0:0x0000 (segment 0x07c0, offset 0), which resolves to the
    ; same physical address, but can be surprising. 
    ; A good practice is to enforce CS:IP at the very start of your boot sector.
    jmp 0x7c0:start

handle_zero:
    mov al, 'A'
    mov ah, 0eh
    mov bx, 0x0
    int 0x10
    iret

handle_one:
    mov al, 'B'
    mov ah, 0eh
    mov bx, 0x0
    int 0x10
    iret
start:

    cli ; int off TODO: should this be performed here or before?
    mov ax, 0x7c0
    
    mov ds, ax ; data segment
    mov es, ax ; special segment
    mov ax, 0x00
    mov ss, ax ; stack segment
    mov sp, 0x7c00 ; stack pointer

    ; set IVT [[offset,segment]_0,[offset, segment]_1, ...] 
    ; IVT is used in real mode, whereas IDT is used in protected mode
    ; int 0 is div-by-zero exception
    mov word[ss:0x00], handle_zero ; make mov use SS segment instead of DS
                                   ; the interrupt vector table starts at 
                                   ; byte 0 of segment
    mov word[ss:0x02], 0x7c0
    
    mov word[ss:0x04], handle_one
    mov word[ss:0x06], 0x7c0

    sti ; int on

    int 1

    mov si, message
    mov byte al, [es:32]
    call print
    jmp $ ;jump to itself

print:
    mov bx, 0
.loop:
    lodsb ; load *(ds:si) into al, then increment si 
    cmp al, 0
    je .done
    call print_char
    jmp .loop
.done:
    ret


print_char:
    mov ah, 0eh
    int 0x10 ;print (bios)
    ret


message:db 'Hello World!', 0 ; message + \0

times 510 - ($ - $$) db 0 ; fill 510 bytes as 0 (510 - (current_addr - begin)))

dw 0xAA55; (little endian, 0x055AA is the boot signatura)
