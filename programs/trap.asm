[bits 32]
section .text
global _program_start

_program_start:
trap:
    jmp trap
