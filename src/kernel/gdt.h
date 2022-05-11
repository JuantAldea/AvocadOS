#ifndef __GDT__H__
#define __GDT__H__
#include <stddef.h>
#include <stdint.h>
#include "../config.h"

struct gdt_native {
    uint16_t segment;
    uint16_t base_addr_0_15_bits;
    uint8_t base_addr_16_23_bits;
    uint8_t access;
    uint8_t high_low_4_bit_flags;
    uint8_t base_addr_24_31_bits;
} __attribute__((packed));

struct gdt_structure {
    uint32_t base;
    uint32_t limit;
    uint8_t type;
};

struct tss {
    uint32_t link;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t sr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldtr;
    uint32_t iopb;
} __attribute__((packed));

extern struct gdt_native gdt_native[GDT_SEGMENTS];
extern struct gdt_structure gdt_segments[GDT_SEGMENTS];
extern struct tss tss;
extern void tss_load(int tss_segment);
void gdt_init();
void tss_set_kernel_stack(uintptr_t kernel_stack_pointer);

#endif
