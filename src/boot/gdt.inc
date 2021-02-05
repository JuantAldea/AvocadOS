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

; offset 0x10
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

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
