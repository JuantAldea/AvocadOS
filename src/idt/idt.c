#include "../config.h"
#include "idt.h"
#include "../memory/memory.h"
#include "../termio/termio.h"

struct idt_desc idt_descriptors [AVOCADOS_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

extern void idt_load (struct idtr_desc *ptr);

void isr_0()
{
    print("Error Division by zero\n");
}

void idt_set(int int_number, void *addr)
{
    struct idt_desc *desc = &(idt_descriptors[int_number]);
    desc->offset_1 = (uint32_t) addr & 0x0000ffff;
    desc->selector = KERNEL_CODE_SELECTOR;
    desc->zero = 0x00;

    desc->attrs.type = 0xE;
    desc->attrs.s = 0;
    desc->attrs.dpl = 3;
    desc->attrs.p = 1;

    //desc->type_attr = 0xEE;
    desc->offset_2 = (uint32_t) addr >> 16;
}


void idt_init()
{
    memset(idt_descriptors, 0, sizeof(idt_descriptors));

    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
    idtr_descriptor.base = (uint32_t) idt_descriptors;

    idt_set(0, isr_0);

    // Load IDT table
    idt_load(&idtr_descriptor);
}