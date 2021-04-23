%include "build/asm_constants.inc"

section .text
global tss_load:function
tss_load:
    push ebp
    mov ebp, esp

    mov ax, [ebp + 8]
    ltr ax

    mov esp, ebp
    pop ebp
    ret
