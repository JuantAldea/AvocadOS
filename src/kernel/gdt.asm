section .text
global gdt_load:function
extern GDT_PTR
gdt_load:
	; Load the GDT
	lgdt [GDT_PTR]
	; Flush the values to 0x10
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	jmp 0x08:flush2
flush2:
	ret
