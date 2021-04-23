[BITS 32]
section .text

%include "build/asm_constants.inc"

global restore_general_registers
global load_user_segments
global task_continue

load_user_segments:
    mov ax, GDT_USER_DATA_SEGMENT_SELECTOR | 0x3 ;  + RPL 3 (ring 3)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret

; void restore_general_registers(struct general_purpose_registers *regs)
restore_general_registers:
    mov ebx, [esp + 8]
    mov edi, [ebx + general_purpose_registers_edi]
    mov esi, [ebx + general_purpose_registers_esi]
    mov ebp, [ebx + general_purpose_registers_ebp]
    mov edx, [ebx + general_purpose_registers_edx]
    mov ecx, [ebx + general_purpose_registers_ecx]
    mov eax, [ebx + general_purpose_registers_eax]
    mov ebx, [ebx + general_purpose_registers_ebx]
    ret

; void task_continue(struct isr_data *data)
task_continue:
    mov ebp, esp

    ; point to *data
    mov ebx, [ebp + 4]

    ; stack selector
    push dword [ebx + interrupt_frame_ss]

    ; stack pointer
    push dword [ebx + interrupt_frame_esp]

    ; push flags enabling interrupts
    pushf
    pop eax
    or eax, 0x200
    push eax

    ; code segment
    push dword [ebx + interrupt_frame_cs]

    ; push eip
    push dword [ebx + interrupt_frame_eip]

    ; segment registers
    mov ax, [ebx + interrupt_frame_ss]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; pass parameter data->interrupt_frame_general_regs
    push dword [ebp + 4 + interrupt_frame_general_regs]
    call restore_general_registers

    ; remove previous push
    add esp, 4

    ; jump to user-mode
    iretd
