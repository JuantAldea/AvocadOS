; http://www.ctyme.com/rbrown.htm
;ORG 0x7c00
BITS 16 ; 16 bit code (for the assembler)
global mul_label
global _start
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

KERNEL_32_BUFFER equ 0x0100000 ; 1MB

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
    mov eax, 1 ; sector 0 is the boot sector, we want to load from sector 1
    mov ecx, 100 ; 100 sectors
    mov edi, KERNEL_32_BUFFER
    call ata_lba_read
    jmp CODE_SEG:KERNEL_32_BUFFER


; eax -> first sector to read from
; ecx -> number of sectors to read
; edi -> buffer addr to store the data
ata_lba_read:
    ; mode 28 bit PIO  -> https://wiki.osdev.org/ATA_PIO_Mode
    pushfd
    and eax, 0x0FFFFFFF ; trucate to 28 bit
    push eax
    push ebx
    push edx
    push ecx
    push edi

    mov ebx, eax ; backup LBA to EBX

    ; TODO: can this consecutive ports be treated at a single one?

    ; send high 8 bits of the LBA to HD controller
    ; OR'd with the drive
    mov edx, 0x01F6 ; select port
    shr eax, 24 ; shift right 24 -> get bytes 24-27 to AL
    or al, 0xE0 ; select Master drive
    out dx, al  ; write to bus

    ; send the total sectors to read
    mov dx, 0x01F2 ; port select
    mov eax, ecx ; set number of sectors to read
    out dx, al   ; write to bus

    ; send bits LBA_0-7
    mov edx, 0x01F3 ; select port
    mov eax, ebx; restore LBA
    ;shr eax, 0 ; move bits 0-7 to AL
    out dx, al  ; write to bus

    ; send bits LBA_8-16
    mov edx, 0x01F4 ; select port
    mov eax, ebx; restore LBA
    shr eax, 8  ; move bits 8-16 to AL
    out dx, al  ; write to bus

    ; send bits LBA_17-23
    mov edx, 0x01F5 ; select port
    mov eax, ebx; restore LBA
    shr eax, 16  ; move bits 17-23 to AL
    out dx, al  ; write to bus

    ; issue read command
    mov edx, 0x01F7 ; command register port
    mov al, 0x20 ; read command
    out dx, al

    mov ebx, ecx ; store in ebx the number of sectors to read

    .wait_for_more:
        mov edx, 0x1F7 ; command register port ; Poll port
        in al, dx
        test al, 8
        jz .wait_for_more


    mov edx, 0x1F0 ; data port
    mov ecx, 256
    rep insw ; I/O
    dec ebx
    jnz .wait_for_more

    pop edi
    pop ecx
    pop edx
    pop ebx
    pop eax
    popfd
    ret

times 510 - ($ - $$) db 0 ; fill 510 bytes as 0 (510 - (current_addr - begin)))

dw 0xAA55; (little endian, 0x055AA is the boot signature)

; BIOS wont load anthing from here, as it loads only 512 bytes
