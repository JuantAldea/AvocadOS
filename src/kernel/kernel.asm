[BITS 32]

%include "build/asm_constants.inc"

KERNEL_VIRTUAL_BASE equ 0xC0000000
KERNEL_BASE_PAGE_NUMBER equ (KERNEL_VIRTUAL_BASE >> 22)
PSE_BIT     equ 0x00000010
PG_BIT      equ 0x80000000

section .data
align 4096
global BOOT_PAGE_DIRECTORY
BOOT_PAGE_DIRECTORY:
    dd 0x00000083
    times (KERNEL_BASE_PAGE_NUMBER - 1) dd 0
    dd 0x00000083
    times (1024 - KERNEL_BASE_PAGE_NUMBER - 1) dd 0

section .data
align 4096
; will store kernel's system task (union task *system_task = &system_task_addr;)
; TODO use linker script instead of this
global system_task
global system_task_stack_bottom
system_task times 4096 dd 0xDEADC0DE
system_task_stack_bottom:

section .text
global kernel_trampoline_low
kernel_trampoline_low equ (kernel_trampoline - KERNEL_VIRTUAL_BASE)
kernel_trampoline:
    mov ecx, (BOOT_PAGE_DIRECTORY - KERNEL_VIRTUAL_BASE)
    mov cr3, ecx

    mov ecx, cr4
    or ecx, PSE_BIT
    mov cr4, ecx

    mov ecx, cr0
    or ecx, PG_BIT
    mov cr0, ecx

    lea ecx, [higher_half]
    jmp ecx

higher_half:
    mov esp, system_task_stack_bottom
    xor ebp, ebp
    ;call remap_master_pic
    ;extern remap_master_pic_C
    ;call remap_master_pic_C

    extern kernel_main
    call kernel_main
    cli
    jmp $

; Programmable Interrupt Controller
; https://pdos.csail.mit.edu/6.828/2018/readings/hardware/8259A.pdf
; https://wiki.osdev.org/PIC#Protected_Mode
;remap_master_pic:
;    push ebp
;    mov ebp, esp
;
;    ; ICW1
;    ; Initialization command in cascade mode (ICW4 needed)
;    mov al, 00010001b ;0x11
;
;    ; initialize PIC1
;    out 0x20, al
;    call io_delay
;    ; initialize PIC2
;    out	0xA0, al
;    call io_delay
;
;    ; ICW2
;    ; A common choice is to move them to the beginning of the available range
;    ; (IRQs 0..0xF -> INT 0x20..0x2F).
;    ; For that, we need to set the master PIC's
;    ; base for PIC1 ->0x20
;    mov al, 0x20
;    out 0x21, al
;    call io_delay
;
;    ; base for PIC2 -> 0x28
;    mov al, 0x28
;    out 0xA1, al
;    call io_delay
;
;    ; ICW3
;    ; configure Master so that it is aware of the Slave
;    ; connects to IRQ2
;    mov al, 00000100b ;
;    out 0x21, al
;    call io_delay
;
;    ; configure Slave cascade identity ("slave ID")
;    mov al, 00000010b ;
;    out 0xA1, al
;    call io_delay
;
;    ; ICW4:
;    ; put master and slave into 8086/88 (MCS-80/85) mode
;    mov al, 00000001b
;    out 0x21, al
;    call io_delay
;    out 0xA1, al
;    call io_delay
;
;    mov esp, ebp
;    pop ebp
;    ret
;
;io_delay:
;	nop
;	nop
;	nop
;	nop
;	ret
; C compiler requires functions to be aligned to 16 bytes
; To ensure this assmebly does not destroy their alignment, as this piece of code
; is located at the begining of the bin file, in the .text section, we pad this to
; 512 bytes, hence preserving the 16 byte aligment of whatever comes after.
; 512 % 16 == 0
;times 512 - ($ - $$) db 0;
; Handled by the linker script
