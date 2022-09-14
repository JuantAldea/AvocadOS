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

extern union task *current_task __attribute__((aligned(PAGING_PAGE_SIZE)));
struct idt_desc idt_descriptors[KERNEL_TOTAL_INTERRUPTS] __attribute__((aligned(PAGING_PAGE_SIZE)));
struct idtr_desc idtr_descriptor __attribute__((aligned(PAGING_PAGE_SIZE)));

typedef void *(*isr_handler)(struct interrupt_frame *interrupt_frame);
isr_handler isr_handlers[KERNEL_TOTAL_INTERRUPTS] __attribute__((aligned(PAGING_PAGE_SIZE)));

#define PIC1 0x20 /* IO base address for master PIC */
#define PIC2 0xA0 /* IO base address for slave PIC */
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)
#define ICW1_ICW4 0x01 /* ICW4 (not) needed */
#define ICW1_SINGLE 0x02 /* Single (cascade) mode */
#define ICW1_INTERVAL4 0x04 /* Call address interval 4 (8) */
#define ICW1_LEVEL 0x08 /* Level triggered (edge) mode */
#define ICW1_INIT 0x10 /* Initialization - required! */

#define ICW4_8086 0x01 /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO 0x02 /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE 0x08 /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C /* Buffered mode/master */
#define ICW4_SFNM 0x10 /* Special fully nested (not) */

void enable_interrupts()
{
    asm volatile("sti");
}

void disable_interrupts()
{
    asm volatile("cli");
}

static inline void pic_ack(int intno)
{
    if (intno >= 40) {
        outb(0xA0, 0x20);
    }

    outb(0x20, 0x20);
}

void *isr_0x0_handler(struct interrupt_frame *interrupt_frame)
{
    (void)interrupt_frame;
    print("DIV0\n");
    return interrupt_frame;
}

void *isr_0x1_handler(struct interrupt_frame *interrupt_frame)
{
    (void)interrupt_frame;

    print("ISR1\n");
    return interrupt_frame;
}

void *isr_0x6_handler(struct interrupt_frame *interrupt_frame)
{
    (void)interrupt_frame;

    print("ISR 6 -> ");
    char buff[100];
    itoa(interrupt_frame->eip, buff);
    print(buff);
    print("\n");
    while (1) {
    }
    return interrupt_frame;
}

void *isr_0xE_handler(struct interrupt_frame *interrupt_frame)
{
    (void)interrupt_frame;
    char buff[100];
    itoa(interrupt_frame->error_code, buff);
    print(buff);
    panic("Int 0xE - Page Fault\n");
    return interrupt_frame;
}

void *int_0x21_handler(struct interrupt_frame *interrupt_frame)
{
    (void)interrupt_frame;
    print("Int 0x21 - Keyboard\n");
    //ACK
    return interrupt_frame;
}

void *int_0x20_handler(struct interrupt_frame *interrupt_frame)
{
    //print("Int 0x20 - Timer interrupt\n");
    //ACK

    //task_store(data);
    schedule(interrupt_frame);
    return (void *)current_task->task.kernel_stack_pointer;
    //return interrupt_frame;
    //return data;
}

void *int_0xD_handler(struct interrupt_frame *interrupt_frame)
{
    panic("GPF");
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
    return interrupt_frame;
}

void *isr_dispatcher(struct interrupt_frame *interrupt_frame)
{
    //enter_kernel();
    //struct interrupt_frame * = (struct interrupt_frame *)esp;
    //uint8_t in_service = insb(0x20);
    //(void)in_service;
    //return esp;
    /*
    print("ISR ");
    char buff[20];
    itoa(interrupt_frame->int_no, buff);
    print(buff);
    print(" ");
    itoa(interrupt_frame->error_code, buff);
    print(buff);
    print(" ");
    itoa(interrupt_frame->eip, buff);
    print(buff);
    print("\n");
    */

    if (interrupt_frame->int_no > 31) {
        pic_ack(interrupt_frame->int_no - 32);
    }

    if (isr_handlers[interrupt_frame->int_no]) {
        interrupt_frame = isr_handlers[interrupt_frame->int_no](interrupt_frame);
    } else {
        print("Unhandled Interrupt!\n");
    }

    return interrupt_frame;
}

void IRQ_set_mask(unsigned char IRQline)
{
    uint16_t port;
    uint8_t value;

    if (IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = insb(port) | (1 << IRQline);
    outb(port, value);
}

void IRQ_clear_mask(unsigned char IRQline)
{
    uint16_t port;
    uint8_t value;

    if (IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = insb(port) & ~(1 << IRQline);
    outb(port, value);
}

void idt_set(int int_number, void (*addr)(), uint8_t type)
{
    struct idt_desc *desc = &(idt_descriptors[int_number]);
    desc->offset_1 = (uintptr_t)addr & 0x0000ffff;
    desc->offset_2 = ((uintptr_t)addr >> 16) & 0x0000ffff;
    desc->selector = GDT_KERNEL_CODE_SEGMENT_SELECTOR;
    desc->zero = 0x00;
    /*
    desc->attrs.type = 0xE;
    desc->attrs.s = 0;
    desc->attrs.dpl = 3;
    desc->attrs.p = 1;
    */
    desc->type_attr = type | 0x60;
}

void idt_init()
{
    memset(idt_descriptors, 0, sizeof(idt_descriptors)); // NOLINT

    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
    idtr_descriptor.base = (uintptr_t)idt_descriptors;

    //extern void no_interrupt();
    extern void (*isr_table[])();
    for (int i = 0; i < 3; ++i) {
        idt_set(i, isr_table[i], INTGATE);
    }

    idt_set(3, isr_table[3], TRAPGATE);
    idt_set(4, isr_table[4], TRAPGATE);

    for (int i = 5; i < 48; ++i) {
        idt_set(i, isr_table[i], INTGATE);
    }

    for (int i = 0; i < KERNEL_TOTAL_INTERRUPTS; i++) {
        isr_handlers[i] = no_int_handler;
    }

    isr_handlers[0] = isr_0x0_handler;
    isr_handlers[1] = isr_0x1_handler;
    isr_handlers[6] = isr_0x6_handler;
    isr_handlers[0xE] = isr_0xE_handler;
    isr_handlers[0x20] = int_0x20_handler;
    isr_handlers[0x21] = int_0x21_handler;
    isr_handlers[0xD] = int_0xD_handler;
    //IRQ_set_mask(0x2f);
    // Load IDT table
    extern void idt_load(struct idtr_desc * ptr);
    idt_load(&idtr_descriptor);
}
