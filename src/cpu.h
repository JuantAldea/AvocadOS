//TODO uintptr_t?
#ifndef __CPU_H__
#define __CPU_H__

#include <stdint.h>
#include <stddef.h>
struct general_purpose_registers {
    // as pushed by pushad.
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
} __attribute__((packed));

struct isr_frame {
    // Pushed by the processor automatically.
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t ss;
} __attribute__((packed));

#endif
