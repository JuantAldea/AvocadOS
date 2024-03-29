
#include "gdt.h"
#include "../kernel/panic.h"
#include "../string/string.h"
#include "../config.h"
struct gdt_native gdt_native[GDT_SEGMENTS] __attribute__((aligned(PAGING_PAGE_SIZE)));
struct gdt_structure gdt_segments[GDT_SEGMENTS] __attribute__((aligned(PAGING_PAGE_SIZE)));
struct tss tss __attribute__((aligned(PAGING_PAGE_SIZE)));
struct gdt_pointer_t GDT_PTR __attribute__((aligned(PAGING_PAGE_SIZE)));

void gdt_structures_to_native_gdts(struct gdt_native *gdt, struct gdt_structure *structure, int total_entries);
void gdt_structure_to_native(uint8_t *target, struct gdt_structure *structure);
extern void gdt_load();
void tss_init();

void gdt_init()
{
    gdt_segments[GDT_NULL_SEGMENT] = (struct gdt_structure){ .base = 0, .limit = 0, .type = 0 }; //kernel code
    gdt_segments[GDT_KERNEL_CODE_SEGMENT] = (struct gdt_structure){
        .base = 0, .limit = 0xffffffff, .type = GDT_FLAG_RING0 | GDT_FLAG_SEGMENT | GDT_FLAG_CODESEG | GDT_FLAG_PRESENT
    }; //kernel code

    gdt_segments[GDT_KERNEL_DATA_SEGMENT] = (struct gdt_structure){
        .base = 0, .limit = 0xffffffff, .type = GDT_FLAG_RING0 | GDT_FLAG_SEGMENT | GDT_FLAG_DATASEG | GDT_FLAG_PRESENT
    }; // kernel data

    gdt_segments[GDT_USER_CODE_SEGMENT] = (struct gdt_structure){
        .base = 0, .limit = 0xffffffff, .type = GDT_FLAG_RING3 | GDT_FLAG_SEGMENT | GDT_FLAG_CODESEG | GDT_FLAG_PRESENT
    }; // user code

    gdt_segments[GDT_USER_DATA_SEGMENT] = (struct gdt_structure){
        .base = 0, .limit = 0xffffffff, .type = GDT_FLAG_RING3 | GDT_FLAG_SEGMENT | GDT_FLAG_DATASEG | GDT_FLAG_PRESENT
    }; // user data

    gdt_segments[GDT_TSS_SEGMENT] = (struct gdt_structure){ .base = (uintptr_t)&tss,
                                                            .limit = sizeof(struct tss) - 1,
                                                            .type = GDT_FLAG_PRESENT | GDT_FLAG_TSS | GDT_FLAG_RING0 }; // TSS

    gdt_structures_to_native_gdts(gdt_native, gdt_segments, GDT_SEGMENTS);
    GDT_PTR.base = (uintptr_t)&gdt_native;
    GDT_PTR.limit = sizeof(gdt_native) - 1;
    gdt_load();
    tss_init();
}

void tss_init()
{
    memset(&tss, 0, sizeof(struct tss));
    extern uintptr_t system_task_stack_bottom;
    tss.esp0 = (uintptr_t)&system_task_stack_bottom;
    tss.ss0 = GDT_KERNEL_DATA_SEGMENT_SELECTOR;
    tss.cs = 0x0b;
    tss.ss = 0x13;
    tss.ds = 0x13;
    tss.es = 0x13;
    tss.fs = 0x13;
    tss.gs = 0x13;
    tss_load(GDT_TSS_SEGMENT_SELECTOR);
}

void tss_set_kernel_stack(uintptr_t kernel_stack_pointer)
{
    tss.esp0 = kernel_stack_pointer;
}

void gdt_structure_to_native(uint8_t *target, struct gdt_structure *structure)
{
    // TODO: Spend some time reading Intel manuals
    // from osdev
    // Check the limit to make sure that it can be encoded
    if ((structure->limit > 65536) && ((structure->limit & 0xFFF) != 0xFFF)) {
        panic("Invalid GDT");
    }

    // byte block granularity
    // 32 bit protected mode
    // 0b0100
    target[6] = 0x40;

    if (structure->limit > 65536) {
        // Adjust granularity if required
        structure->limit >>= 12;
        // 0b1100 -> page granularity if too big
        target[6] = 0xC0;
    }

    // Encode the limit
    target[0] = structure->limit & 0xFF;
    target[1] = (structure->limit >> 8) & 0xFF;
    target[6] |= (structure->limit >> 16) & 0xF;

    // Encode the base
    target[2] = structure->base & 0xFF;
    target[3] = (structure->base >> 8) & 0xFF;
    target[4] = (structure->base >> 16) & 0xFF;
    target[7] = (structure->base >> 24) & 0xFF;

    // And... Type
    target[5] = structure->type;
}

void gdt_structures_to_native_gdts(struct gdt_native *gdt, struct gdt_structure *structure, int total_entries)
{
    for (int i = 0; i < total_entries; ++i) {
        gdt_structure_to_native((uint8_t *)&gdt[i], &structure[i]);
    }
}
