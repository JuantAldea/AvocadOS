[BITS 32]
global _start_kernel ; export symbol

extern kernel_main ; "forward-declaration" of kernel-start (kernel.c)

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
    call kernel_main;
    jmp $ ;trap


; C compiler requires functions to be aligned to 16 bytes
; To ensure this assmebly does not destroy their alignment, as this piece of code
; is located at the begining of the bin file, in the .text section, we pad this to
; 512 bytes, hence preserving the 16 byte aligment of whatever comes after.
; 512 % 16 == 0
times 512 - ($ - $$) db 0;
