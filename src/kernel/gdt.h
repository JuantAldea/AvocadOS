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

extern struct gdt_native gdt_native[GDT_SEGMENTS];
extern struct tss tss;
extern struct gdt_structure gdt_segments[GDT_SEGMENTS];

void gdt_segments_init_and_load();

void gdt_load(struct gdt_native *gdt, int size);

#endif
