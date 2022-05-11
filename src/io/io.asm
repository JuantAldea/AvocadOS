[BITS 32]
section .text

global insb:function
global insw:function
global outb:function
global outw:function
global io_delay:function

insb:
    ;create stack frame
    push ebp
    mov ebp, esp

    xor eax, eax
    mov edx, [ebp + 8]
    in al, dx

    mov esp, ebp
    pop ebp
    ret

insw:
    push ebp
    mov ebp, esp
    xor eax, eax
    mov edx, [ebp + 8]
    in ax, dx

    mov esp, ebp
    pop ebp
    ret

outb:
    push ebp
    mov ebp, esp
    ;push eax
    ;push edx

    mov eax, [ebp + 12]
    mov edx, [ebp + 8]
    out dx, al

    ;pop edx
    ;pop eax
    mov esp, ebp
    pop ebp
    ret

outw:
    push ebp
    mov ebp, esp
    ;push eax
    ;push edx
    mov eax, [ebp + 12]
    mov edx, [ebp + 8]

    out dx, ax

    ;pop edx
    ;pop eax
    mov esp, ebp
    pop ebp
    ret


io_delay:
	nop
	nop
	nop
	nop
	ret