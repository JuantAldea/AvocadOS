#ifndef __IDT_H
#define __IDT_H

#include <stdint.h>
#include "../cpu.h"
extern void (*raise_int_table[])();

#define INTGATE 0x8E
#define TRAPGATE 0xEF

struct idt_desc {
    uint16_t offset_1; // offset bits 0-15
    uint16_t selector;
    uint8_t zero; // empty
    //uint8_t type_attr;

    union {
        struct {
            uint8_t s : 1;
            uint8_t dpl : 2;
            uint8_t p : 1;
            uint8_t type : 4;
        } __attribute__((packed)) attrs;
        uint8_t type_attr;
    } __attribute__((packed));

    uint16_t offset_2; // offset bits 16-31
} __attribute__((packed));

struct idtr_desc {
    uint16_t limit; //table length - 1 byte
    uintptr_t base; //base pointer of the IDT
} __attribute__((packed));

struct segment_registers {
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
    uint32_t cs;
    uint32_t ss;
} __attribute__((packed));

struct process_state {
    struct segment_registers segments;
    uint32_t eflags;
    uint32_t eip;
    struct general_purpose_registers general_regs;
} __attribute__((packed));

void idt_init();
extern void enable_interrupts();
extern void disable_interrupts();
extern void raise_int_0();
extern void raise_int_1();
extern void raise_int_2();
extern void raise_int_3();
extern void raise_int_32();
extern void raise_int_20();
extern void raise_int_33();

#endif /* __IDT_H*/
