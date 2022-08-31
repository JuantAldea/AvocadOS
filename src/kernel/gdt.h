#ifndef __GDT__H__
#define __GDT__H__
#include <stddef.h>
#include <stdint.h>
#include "../config.h"

struct gdt_native {
    uint16_t limit_low;
    uint16_t base_addr_0_15_bits;
    uint8_t base_addr_16_23_bits;
    uint8_t access;
    uint8_t limit_high_4_flags_4;
    uint8_t base_addr_24_31_bits;
} __attribute__((packed));

struct gdt_structure {
    uint32_t base;
    uint32_t limit;
    uint8_t type;
};

struct gdt_pointer_t {
	uint16_t limit;
	uintptr_t base;
} __attribute__((packed)) ;

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


/// This segment is a data segment
#define GDT_FLAG_DATASEG 0x02
/// This segment is a code segment
#define GDT_FLAG_CODESEG 0x0a
#define GDT_FLAG_TSS 0x09
#define GDT_FLAG_TSS_BUSY 0x02

#define GDT_FLAG_SEGMENT 0x10
/// Privilege level: Ring 0
#define GDT_FLAG_RING0 0x00
/// Privilege level: Ring 1
#define GDT_FLAG_RING1 0x20
/// Privilege level: Ring 2
#define GDT_FLAG_RING2 0x40
/// Privilege level: Ring 3
#define GDT_FLAG_RING3 0x60
/// Segment is present
#define GDT_FLAG_PRESENT 0x80
/// Segment was accessed

#define GDT_FLAG_ACCESSED 0x01
#define GDT_FLAG_4K_GRAN 0x80
#define GDT_FLAG_16_BIT 0x00
#define GDT_FLAG_32_BIT 0x40
#define GDT_FLAG_64_BIT 0x20


#endif
