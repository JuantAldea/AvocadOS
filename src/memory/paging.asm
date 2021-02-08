section .text
[BITS 32]
[GLOBAL paging_load_directory]
[GLOBAL enable_paging]

paging_load_directory:
    push ebp
    mov ebp, esp

    mov eax, [ebp + 8]
    mov cr3, eax

    pop ebp
    ret

enable_paging:
    push ebp
    mov ebp, esp

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    pop ebp
    ret
