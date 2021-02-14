#ifndef __IDT_H
#define __IDT_H

#include <stdint.h>

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
    uint32_t base; //base pointer of the IDT
    //struct idt_desc *base; //base pointer of the IDT

} __attribute__((packed));

void idt_init();
void enable_interrupts();
void disable_interrupts();
extern void raise_int_0x0();
extern void raise_int_0x1();
extern void raise_int_0x2();
extern void raise_int_0x3();
extern void raise_int_0x20();
extern void raise_int_0x21();

#endif /* __IDT_H*/
