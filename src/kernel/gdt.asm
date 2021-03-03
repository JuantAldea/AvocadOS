section .text
global gdt_load

gdt_load:
    push ebp
    mov ebp, esp

    mov eax, [ebp + 8]
    mov [gdt_descriptor + 2], eax
    mov ax, [ebp + 12]
    mov [gdt_descriptor], ax
    lgdt [gdt_descriptor]

    mov	esp, ebp
    pop ebp
    ret

section .data

gdt_descriptor:
    dw 0x00 ; size
    dd 0x00 ; gdt ptr
