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

start:

    cli ; int off TODO: should this be performed here or before?
    mov ax, 0x7c0
    
    mov ds, ax ; data segment
    mov es, ax ; special segment
    mov ax, 0x00
    mov ss, ax ; stack segment
    mov sp, 0x7c00 ; stack pointer
    sti ; int on

    ; int 0x13/ah=0x2-> read data from disk using CHS addressing
    mov ah, 2 ; read sector command
    mov al, 1 ; one sector
    mov ch, 0 ; cylinder low 8 bits
    mov cl, 2 ; read second sector, as the message is loaded from byte 512. For CHS addressing, sectors start at 1
    mov dh, 0 ; head number
    ; mov dl, XXX ; there's no need to set the Drive Number as the bios sets it for us to the one we are booting from
    mov bx, buffer ; ES register (extra segment) already points to 0x7c0
    int 0x13 ;invoke ISR 13
    jc error
    mov si, buffer
    call print
    jmp $ ; trap

error:
    mov si, error_message
    call print
    jmp $ ;trap

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


error_message:db 'Failed to load sector', 0 ; message + \0

times 510 - ($ - $$) db 0 ; fill 510 bytes as 0 (510 - (current_addr - begin)))

dw 0xAA55; (little endian, 0x055AA is the boot signature)

; BIOS wont load anthing from here, as it loads only 512 bytes
buffer: ; nothing here, just a buffer
