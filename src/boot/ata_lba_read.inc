; eax -> first sector to read from
; ecx -> number of sectors to read
; edi -> buffer addr to store the data
ata_lba_read:
    push ebp
    mov ebp, esp
    ; mode 28 bit PIO  -> https://wiki.osdev.org/ATA_PIO_Mode
    pushfd
    and eax, 0x0FFFFFFF ; trucate to 28 bits
    push eax
    push ebx
    push edx
    push ecx
    push edi

    mov ebx, eax ; backup LBA to EBX
    ; TODO: can this consecutive ports be treated at a single one?

    ; send high 8 bits of the LBA to HD controller OR'd with the drive
    ; which right now is hardcoded (0xE0) to the MASTER drive
    mov edx, 0x01F6 ; select port
    shr eax, 24 ; shift right 24 -> get bytes 24-27 to AL
    or al, 0xE0 ; select Master drive
    out dx, al  ; write to bus

    ; send the total sectors to read
    mov dx, 0x01F2 ; port select
    mov eax, ecx ; set number of sectors to read (uint8_t)
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

    ; a 400ns wait is needed https://wiki.osdev.org/ATA_PIO_Mode#400ns_delays
    .read_next:
    .wait:
        mov edx, 0x1F7 ; command register port ; Poll port
        in al, dx ; get status byte
        test al, 0x80 ; test for BSY bit
        jnz short .wait
        test al, 0x8 ; test for DRQ bit, if set we are good to go
    .retry:
        jz short .wait

    mov edx, 0x1F0 ; data port
    mov ecx, 256
    rep insw ; I/O
    in al, dx	 	; delay 400ns to allow drive to set new values of BSY and DRQ
    in al, dx
    in al, dx
    in al, dx
    dec ebx
    jnz short .read_next

    pop edi
    pop ecx
    pop edx
    pop ebx
    pop eax
    popfd
    pop ebp
    ret
