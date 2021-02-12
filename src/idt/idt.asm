[BITS 32]
section .text

global idt_load

idt_load:
    push ebp
    mov ebp, esp
    push ebx

    mov ebx, [ebp + 8]
    lidt [ebx]

    pop ebx
    pop ebp
    ret

%macro GENERATE_ISR_ROUTINES 1  ; macro, one parameter
    GENERATE_RAISE_INT %1
    ISR %1
%endmacro

%macro GENERATE_RAISE_INT 1  ; macro, one parameter
    [GLOBAL raise_int_%1]
    raise_int_%1:
        int %1
        ret
%endmacro

%macro ISR 1  ; macro, one parameter
    [GLOBAL isr_%1]
    isr_%1:
        cli
        push byte 0
        push byte %1
        jmp isr_wrapper
%endmacro

extern isr_dispatcher

isr_wrapper:
    pushad                    ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax

    mov ax, ds               ; Lower 16-bits of eax = ds.
    push eax                 ; save the data segment descriptor

    mov ax, 0x10  ; load the kernel data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call isr_dispatcher

    pop eax        ; reload the original data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popad  ; Pops edi,esi,ebp...
    add esp, 8     ; Cleans up the pushed error code and pushed ISR number
    sti
    iret           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP

GENERATE_ISR_ROUTINES 0x0
GENERATE_ISR_ROUTINES 0x1
GENERATE_ISR_ROUTINES 0x2
GENERATE_ISR_ROUTINES 0x3
GENERATE_ISR_ROUTINES 0x4
GENERATE_ISR_ROUTINES 0x20
GENERATE_ISR_ROUTINES 0x21

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
