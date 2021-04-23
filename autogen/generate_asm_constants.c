#include <stddef.h>
#include "../src/idt/idt.h"
#include "../src/kernel/gdt.h"
#include "../src/cpu.h"

// https://www.avrfreaks.net/sites/default/files/avrbeginners_30_Accessing_C_Structs_in_Assembler_1.0.1.pdf
#define _ASMDEFINE(sym, offset) asm volatile("\n-> " #sym " %0 \n" ::"i"(offset))
#define ASMDEFINE(s, m) _ASMDEFINE(s##_##m, offsetof(s, m));

void isr_frame_generate_offsets()
{
    ASMDEFINE(struct interrupt_frame, general_regs);
    ASMDEFINE(struct interrupt_frame, int_no);
    ASMDEFINE(struct interrupt_frame, error_code);
    ASMDEFINE(struct interrupt_frame, eip);
    ASMDEFINE(struct interrupt_frame, cs);
    ASMDEFINE(struct interrupt_frame, eflags);
    ASMDEFINE(struct interrupt_frame, esp);
    ASMDEFINE(struct interrupt_frame, ss);
    ASMDEFINE(struct interrupt_frame, gs);
    ASMDEFINE(struct interrupt_frame, fs);
    ASMDEFINE(struct interrupt_frame, es);
    ASMDEFINE(struct interrupt_frame, ds);
}

void general_purpose_registers_generate_offsets()
{
    ASMDEFINE(struct general_purpose_registers, edi);
    ASMDEFINE(struct general_purpose_registers, esi);
    ASMDEFINE(struct general_purpose_registers, ebp);
    ASMDEFINE(struct general_purpose_registers, esp);
    ASMDEFINE(struct general_purpose_registers, ebx);
    ASMDEFINE(struct general_purpose_registers, edx);
    ASMDEFINE(struct general_purpose_registers, ecx);
    ASMDEFINE(struct general_purpose_registers, eax);
}

#define _ASMDEFINE_GDT_SEGMENT_OFFSET(sym, offset) asm volatile("\n-> FILLER " #sym " %0 \n" ::"i"(offset))
#define ASMDEFINE_GDT_SEGMENT_OFFSET(s) _ASMDEFINE_GDT_SEGMENT_OFFSET(s##_SELECTOR, s * sizeof(struct gdt_native));

void gdt_offsets()
{
    // ASMDEFINE_GDT_SEGMENT_OFFSET(GDT_NULL_SEGMENT_INDEX);
    ASMDEFINE_GDT_SEGMENT_OFFSET(GDT_KERNEL_CODE_SEGMENT);
    ASMDEFINE_GDT_SEGMENT_OFFSET(GDT_KERNEL_DATA_SEGMENT);
    ASMDEFINE_GDT_SEGMENT_OFFSET(GDT_USER_CODE_SEGMENT);
    ASMDEFINE_GDT_SEGMENT_OFFSET(GDT_USER_DATA_SEGMENT);
    ASMDEFINE_GDT_SEGMENT_OFFSET(GDT_TSS_SEGMENT);
}
