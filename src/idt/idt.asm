[BITS 32]

%include "build/asm_constants.inc"

section .text

global idt_load
global load_kernel_segments

idt_load:
    push ebp
    mov ebp, esp
    push ebx

    mov ebx, [ebp + 8]
    lidt [ebx]

    pop ebx
    pop ebp
    ret

%macro GENERATE_ISR_NO_ERROR_CODE_ROUTINES 1  ; macro, one parameter
    GENERATE_RAISE_INT %1
    ISR_NO_ERROR_CODE %1
%endmacro

%macro GENERATE_ISR_ERROR_CODE_ROUTINES 1  ; macro, one parameter
    GENERATE_RAISE_INT %1
    ISR_ERROR_CODE %1
%endmacro

%macro GENERATE_RAISE_INT 1  ; macro, one parameter
    [GLOBAL raise_int_%1]
    raise_int_%1:
        int %1
        ret
%endmacro

%macro ISR_NO_ERROR_CODE 1  ; macro, one parameter
    [GLOBAL isr_%1]
    isr_%1:
        cli
        push byte 0
        push byte %1
        jmp isr_wrapper
%endmacro

%macro ISR_ERROR_CODE 1  ; macro, one parameter
    [GLOBAL isr_%1]
    isr_%1:
        cli
        push byte %1
        jmp isr_wrapper
%endmacro

extern isr_dispatcher

isr_wrapper:
    ; pushed so far
    ; by cpu:
    ;   ss, esp, eflags, cs, eip
    ; by us:
    ;   errcode, isr number

    ; Kernel -> kernel
    ;   SS: unchanged
    ;   ESP new frame pushed
    ;   CS:EIP from IDT

    ; User -> kernel
    ;   SS:ESP TSS { ss0:esp0}
    ;   CS:EIP from IDT
    ;   EFLAGS:

    pushad                    ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax
    ; save previous segments
    push ds
    push es
    push fs
    push gs
    ; cs is pushed by the CPU

    ; switch to kernel segments

    push esp

    call isr_dispatcher
    add esp, 4

    ; restore segments
    pop gs
    pop fs
    pop es
    pop ds

    ; restore registers
    popad  ; Pops pushad
    add esp, 8     ; Cleans up the pushed error code and pushed ISR number
    sti
    iret           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP

GENERATE_ISR_NO_ERROR_CODE_ROUTINES 0x0
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 0x1
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 0x2
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 0x3
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 0x4
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 0x20
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 0x21
GENERATE_ISR_ERROR_CODE_ROUTINES 0xE

[GLOBAL no_interrupt]
[EXTERN no_int_handler]
no_interrupt:
    cli
    pushad
    call no_int_handler
    popad
    sti
    iret

[GLOBAL enable_interrupts]
enable_interrupts:
    sti
    ret

[GLOBAL disable_interrupts]
disable_interrupts:
    cli
    ret

load_kernel_segments:
    mov ax, GDT_KERNEL_DATA_SEGMENT_SELECTOR  ; load the kernel data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret
