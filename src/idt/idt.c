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

struct idt_desc idt_descriptors[KERNEL_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

typedef void *(*isr_handler)(struct isr_data *data);
isr_handler isr_handlers[KERNEL_TOTAL_INTERRUPTS];

void *isr_0x0_handler(struct isr_data *data)
{
    (void)data;
    print("DIV0\n");
    outb(0x20, 0x20);
    return data;
}

void *isr_0x1_handler(struct isr_data *data)
{
    (void)data;

    print("ISR1\n");
    outb(0x20, 0x20);
    return data;
}

void *isr_0xE_handler(struct isr_data *data)
{
    (void)data;
    panic("Int 0xE - Page Fault\n");
    outb(0x20, 0x20);
    return data;
}

void *int_0x21_handler(struct isr_data *data)
{
    (void)data;
    print("Int 0x21 - Keyboard\n");
    //ACK
    outb(0x20, 0x20);
    return data;
}

void *int_0x20_handler(struct isr_data *data)
{
    (void)data;
    print("Int 0x20 - Timer interrupt\n");
    //ACK
    outb(0x20, 0x20);
    return data;
}

void *no_int_handler(struct isr_data *data)
{
    (void)data;
    outb(0x20, 0x0B);
    uint8_t in_service = insb(0x20);
    (void)in_service;
    print("No Int handler\n");
    outb(0x20, 0x20);
    return data;
}

void enter_kernel()
{
    extern void load_kernel_segments();
    load_kernel_segments();
    paging_switch_directory(&kernel_page_directory);
}

void *isr_dispatcher(struct isr_data *data)
{
    //enter_kernel();

    outb(0x20, 0x0B);
    uint8_t in_service = insb(0x20);
    (void)in_service;

    isr_handlers[data->int_no](data);

    /*
    switch (data->int_no) {
    case 0x0:
        isr_0x0_handler();
        break;
    case 0x1:
        isr_0x1_handler();
        break;
    case 0xE:
        isr_0xE_handler();
        break;
    case 0x20:
        int_0x20_handler();
        break;
    case 0x21:
        int_0x21_handler();
        break;
    default:
        no_int_handler(data->int_no);
        break;
    }*/

    //enter_user();
    //outb(0x20, 0x20);
    return data;
}

void idt_set(int int_number, void *addr)
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
    extern void isr_0x20();

    for (int i = 0; i < KERNEL_TOTAL_INTERRUPTS; ++i) {
        idt_set(i, no_interrupt);
    }

    idt_set(0x0, isr_0x0);
    idt_set(0x1, isr_0x1);
    idt_set(0x20, isr_0x20);
    idt_set(0x21, isr_0x21);
    idt_set(0xE, isr_0xE);

    for (int i = 0; i < KERNEL_TOTAL_INTERRUPTS; i++) {
        isr_handlers[i] = no_int_handler;
    }

    isr_handlers[0] = isr_0x0_handler;
    isr_handlers[1] = isr_0x1_handler;
    isr_handlers[0xE] = isr_0xE_handler;
    isr_handlers[0x20] = int_0x20_handler;
    isr_handlers[0x21] = int_0x21_handler;

    // Load IDT table
    extern void idt_load(struct idtr_desc * ptr);
    idt_load(&idtr_descriptor);
}
