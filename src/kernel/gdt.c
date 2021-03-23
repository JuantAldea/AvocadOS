
#include "gdt.h"
#include "../kernel/panic.h"
#include "../string/string.h"

#include "task/tss.h"

struct gdt_native gdt_native[GDT_SEGMENTS];
struct gdt_structure gdt_segments[GDT_SEGMENTS];
void gdt_structures_to_native_gdts(struct gdt_native *gdt, struct gdt_structure *structure, int total_entries);
void gdt_structure_to_native(uint8_t *target, struct gdt_structure *structure);

void gdt_segments_init_and_load()
{
    gdt_segments[GDT_NULL_SEGMENT] = (struct gdt_structure){ .base = 0, .limit = 0, .type = 0 }; //kernel code
    gdt_segments[GDT_KERNEL_CODE_SEGMENT] = (struct gdt_structure){ .base = 0, .limit = 0xffffffff, .type = 0x9a }; //kernel code
    gdt_segments[GDT_KERNEL_DATA_SEGMENT] = (struct gdt_structure){ .base = 0, .limit = 0xffffffff, .type = 0x92 }; // kernel data
    gdt_segments[GDT_USER_CODE_SEGMENT] = (struct gdt_structure){ .base = 0, .limit = 0xffffffff, .type = 0xf8 }; // user code
    gdt_segments[GDT_USER_DATA_SEGMENT] = (struct gdt_structure){ .base = 0, .limit = 0xffffffff, .type = 0xf2 }; // user data
    gdt_segments[GDT_TSS_SEGMENT] = (struct gdt_structure){ .base = (uintptr_t)&tss, .limit = sizeof(tss), .type = 0xE9 }; // TSS

    memset(&tss, 0, sizeof(tss)); //NOLINT

    gdt_structures_to_native_gdts(gdt_native, gdt_segments, GDT_SEGMENTS);
    gdt_load(gdt_native, sizeof(gdt_native));
}

void gdt_structure_to_native(uint8_t *target, struct gdt_structure *structure)
{
    // from osdev
    // Check the limit to make sure that it can be encoded
    if ((structure->limit > 65536) && ((structure->limit & 0xFFF) != 0xFFF)) {
        panic("Invalid GDT");
    }

    target[6] = 0x40;

    if (structure->limit > 65536) {
        // Adjust granularity if required
        structure->limit = structure->limit >> 12;
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
