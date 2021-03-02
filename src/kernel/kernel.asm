[BITS 32]
section .text
[GLOBAL _start_kernel] ; export symbol
[EXTERN kernel_main] ; "forward-declaration" of kernel_main (kernel.c)

CODE_SEG equ 0x08
DATA_SEG equ 0x10

_start_kernel:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ; set stack pointer
    mov ebp, 0x00200000
    mov esp, ebp

    call remap_master_pic
    xor ebp, ebp ; requiered for stack unwinding
    call kernel_main;

    jmp $ ;trap

; Programmable Interrupt Controller
remap_master_pic:
    push ebp
    mov ebp, esp

    mov al, 00010001b
    out 0x20, al

    mov al, 0x20 ; int 0x20
    out 0x21, al

    mov al, 00000001b
    out 0x21, al

    mov esp, ebp
    pop ebp
    ret

; C compiler requires functions to be aligned to 16 bytes
; To ensure this assmebly does not destroy their alignment, as this piece of code
; is located at the begining of the bin file, in the .text section, we pad this to
; 512 bytes, hence preserving the 16 byte aligment of whatever comes after.
; 512 % 16 == 0
;times 512 - ($ - $$) db 0;
