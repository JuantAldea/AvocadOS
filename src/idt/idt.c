#include "../config.h"
#include "idt.h"
#include "../string/string.h"
#include "../termio/termio.h"
#include "../io/io.h"
#include "../cpu.h"
#include "../kernel/gdt.h"
#include "../memory/paging.h"
#include "../kernel/kernel.h"
#include "../kernel/task/process.h"
#include "../kernel/panic.h"
#include "../kernel/task/task.h"

extern union task *current_task;
struct idt_desc idt_descriptors[KERNEL_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

typedef void *(*isr_handler)(struct interrupt_frame *interrupt_frame);
isr_handler isr_handlers[KERNEL_TOTAL_INTERRUPTS];

void *isr_0x0_handler(struct interrupt_frame *interrupt_frame)
{
    (void)interrupt_frame;
    print("DIV0\n");
    outb(0x20, 0x20);
    return interrupt_frame;
}

void *isr_0x1_handler(struct interrupt_frame *interrupt_frame)
{
    (void)interrupt_frame;

    print("ISR1\n");
    outb(0x20, 0x20);
    return interrupt_frame;
}

void *isr_0xE_handler(struct interrupt_frame *interrupt_frame)
{
    (void)interrupt_frame;
    panic("Int 0xE - Page Fault\n");
    outb(0x20, 0x20);
    return interrupt_frame;
}

void *int_0x21_handler(struct interrupt_frame *interrupt_frame)
{
    (void)interrupt_frame;
    print("Int 0x21 - Keyboard\n");
    //ACK
    outb(0x20, 0x20);
    return interrupt_frame;
}

void *int_0x20_handler(struct interrupt_frame *interrupt_frame)
{
    //print("Int 0x20 - Timer interrupt\n");
    //ACK
    outb(0x20, 0x20);
    //task_store(data);
    schedule(interrupt_frame);
    return (void *)current_task->task.kernel_stack_pointer;
    //return data;
}

void *int_0xD_handler(struct interrupt_frame *interrupt_frame)
{
    panic("GPF");
    outb(0x20, 0x20);
    return NULL;
}


void *no_int_handler(struct interrupt_frame *interrupt_frame)
{
    (void)interrupt_frame;
    outb(0x20, 0x0B);
    uint8_t in_service = insb(0x20);
    (void)in_service;
    char buffer[20] = { 0 };
    itoa(interrupt_frame->int_no, buffer);
    print("No Int handler -> ");
    print(buffer);
    print(" \n");
    outb(0x20, 0x20);
    return interrupt_frame;
}

void *isr_dispatcher(struct interrupt_frame *interrupt_frame)
{
    //enter_kernel();

    outb(0x20, 0x0B);
    uint8_t in_service = insb(0x20);
    (void)in_service;

    return isr_handlers[interrupt_frame->int_no](interrupt_frame);
}

void idt_set(int int_number, void (*addr)())
{
    struct idt_desc *desc = &(idt_descriptors[int_number]);
    desc->offset_1 = (uintptr_t)addr & 0x0000ffff;
    desc->selector = GDT_KERNEL_CODE_SEGMENT_SELECTOR;
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
    extern void isr_0xE();
    extern void isr_0xD();
    extern void isr_0x20();

    for (int i = 0; i < KERNEL_TOTAL_INTERRUPTS; ++i) {
        idt_set(i, no_interrupt);
    }

    idt_set(0x0, isr_0x0);
    idt_set(0x1, isr_0x1);
    idt_set(0x20, isr_0x20);
    idt_set(0x21, isr_0x21);
    idt_set(0xE, isr_0xE);
    idt_set(0xD, isr_0xD);

    for (int i = 0; i < KERNEL_TOTAL_INTERRUPTS; i++) {
        isr_handlers[i] = no_int_handler;
    }

    isr_handlers[0] = isr_0x0_handler;
    isr_handlers[1] = isr_0x1_handler;
    isr_handlers[0xE] = isr_0xE_handler;
    isr_handlers[0x20] = int_0x20_handler;
    isr_handlers[0x21] = int_0x21_handler;
    isr_handlers[0xD] = int_0xD_handler;

    // Load IDT table
    extern void idt_load(struct idtr_desc * ptr);
    idt_load(&idtr_descriptor);
}
