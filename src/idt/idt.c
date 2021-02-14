#include "../config.h"
#include "idt.h"
#include "../string/string.h"
#include "../termio/termio.h"
#include "../io/io.h"

struct idt_desc idt_descriptors[KERNEL_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

void isr_0x0_handler()
{
    print("DIV0\n");
}

void isr_0x1_handler()
{
    print("ISR1\n");
}

void int_0x21_handler()
{
    print("Int 0x21 - Keyboard\n");
    //ACK
    outb(0x20, 0x20);
}

void int_0x20_handler()
{
    //print("Int 0x20 - Timer interrupt\n");
    //ACK
    outb(0x20, 0x20);
}

void no_int_handler(uint32_t int_no)
{
    (void)int_no;
    //print("No Int handler\n");
    outb(0x20, 0x20);
}

//TODO uintptr_t?
struct registers {
    // Data segment selector
    uint32_t ds;
    // Pushed by pusha.
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    // Interrupt number and error code (if applicable)
    uint32_t int_no, err_code;
    // Pushed by the processor automatically.
    uint32_t eip, cs, eflags, useresp, ss;
};

void isr_dispatcher(struct registers regs)
{
    switch (regs.int_no) {
    case 0x0:
        isr_0x0_handler();
        break;
    case 0x1:
        isr_0x1_handler();
        break;
    case 0x20:
        int_0x20_handler();
        break;
    case 0x21:
        int_0x21_handler();
        break;
    default:
        no_int_handler(regs.int_no);
        break;
    }
}

void idt_set(int int_number, void *addr)
{
    struct idt_desc *desc = &(idt_descriptors[int_number]);
    desc->offset_1 = (uintptr_t)addr & 0x0000ffff;
    desc->selector = KERNEL_CODE_SELECTOR;
    desc->zero = 0x00;

    desc->attrs.type = 0xE;
    desc->attrs.s = 0;
    desc->attrs.dpl = 3;
    desc->attrs.p = 1;

    //desc->type_attr = 0xEE;
    desc->offset_2 = (uintptr_t)addr >> 16;
}

void idt_init()
{
    memset(idt_descriptors, 0, sizeof(idt_descriptors)); // NOLINT

    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
    idtr_descriptor.base = (uintptr_t)idt_descriptors;

    extern void no_interrupt();
    extern void isr_0x0();
    extern void isr_0x1();
    extern void isr_0x21();
    extern void isr_0x20();

    for (int i = 0; i < KERNEL_TOTAL_INTERRUPTS; ++i) {
        idt_set(i, no_interrupt);
    }

    idt_set(0x0, isr_0x0);
    idt_set(0x1, isr_0x1);
    idt_set(0x20, isr_0x20);
    idt_set(0x21, isr_0x21);

    // Load IDT table
    extern void idt_load(struct idtr_desc * ptr);
    idt_load(&idtr_descriptor);
}
