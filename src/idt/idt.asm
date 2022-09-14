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
    ;   (ss, esp,) eflags, cs, eip
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
    cld
    pushad                    ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax
    ; save previous segments
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; isr frame on the stack, push its pointer to isr_dispatcher
    ;xchg bx, bx
    push esp

    xchg bx, bx
    call isr_dispatcher
    ;mov esp, eax
    pop esp
    ;xchg bx, bx
    ;add esp, 4

    ; restore segments
    pop gs
    pop fs
    pop es
    pop ds

    ; restore registers
    popad  ; Pops pushad
    add esp, 8     ; Cleans up the pushed error code and ISR number
    sti
    iret           ; pops 3(5) things at once: CS, EIP, EFLAGS, (SS, and ESP)


;GENERATE_ISR_NO_ERROR_CODE_ROUTINES 0x0
;GENERATE_ISR_NO_ERROR_CODE_ROUTINES 0x1
;GENERATE_ISR_NO_ERROR_CODE_ROUTINES 0x2
;GENERATE_ISR_NO_ERROR_CODE_ROUTINES 0x3
;GENERATE_ISR_NO_ERROR_CODE_ROUTINES 0x4
;GENERATE_ISR_NO_ERROR_CODE_ROUTINES 0x20
;GENERATE_ISR_NO_ERROR_CODE_ROUTINES 0x21
;GENERATE_ISR_ERROR_CODE_ROUTINES 0xE
;GENERATE_ISR_ERROR_CODE_ROUTINES 0xD


GENERATE_ISR_NO_ERROR_CODE_ROUTINES 0
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 1
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 2
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 3
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 4
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 5
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 6
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 7
GENERATE_ISR_ERROR_CODE_ROUTINES    8
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 9
GENERATE_ISR_ERROR_CODE_ROUTINES    10
GENERATE_ISR_ERROR_CODE_ROUTINES    11
GENERATE_ISR_ERROR_CODE_ROUTINES    12
GENERATE_ISR_ERROR_CODE_ROUTINES    13
GENERATE_ISR_ERROR_CODE_ROUTINES    14
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 15
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 16
GENERATE_ISR_ERROR_CODE_ROUTINES    17
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 18
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 19
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 20
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 21
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 22
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 23
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 24
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 25
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 26
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 27
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 28
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 29
GENERATE_ISR_ERROR_CODE_ROUTINES    30
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 31
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 32
GENERATE_ISR_NO_ERROR_CODE_ROUTINES 33

%assign i 34
%rep    94
    GENERATE_ISR_NO_ERROR_CODE_ROUTINES i
%assign i i+1
%endrep



[GLOBAL isr_table]
isr_table:
%assign i 0
%rep    128
    dd isr_%+i ; use DQ instead if targeting 64-bit
%assign i i+1
%endrep

[GLOBAL raise_int_table]
raise_int_table:
%assign i 0
%rep    128
    dd raise_int_%+i ; use DQ instead if targeting 64-bit
%assign i i+1
%endrep

[GLOBAL no_interrupt]
[EXTERN no_int_handler]
no_interrupt:
    cli
    pushad
    call no_int_handler
    popad
    sti
    iret

;[GLOBAL enable_interrupts]
;enable_interrupts:
    ;xchg bx, bx
;    sti
;    ret

;[GLOBAL disable_interrupts]
;disable_interrupts:
    ;xchg bx, bx
;    cli
;    ret
