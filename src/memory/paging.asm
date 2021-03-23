[BITS 32]
section .text
[GLOBAL paging_load_directory]
[GLOBAL enable_paging]

paging_load_directory:
    push ebp
    mov ebp, esp

    mov eax, [ebp + 8]
    mov cr3, eax

    mov esp, ebp
    pop ebp
    ret

enable_paging:
    push ebp
    mov ebp, esp

    ; disable 4M pages
    mov eax, cr4
    xor eax, 0x00000010
    mov cr4, eax

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    mov esp, ebp
    pop ebp
    ret
