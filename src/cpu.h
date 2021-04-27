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

// clang-format off
#define INTERRUPT_FRAME_COMMON \
    /* pushed by isr_wrapper */ \
    uintptr_t gs; \
    uintptr_t fs; \
    uintptr_t es; \
    uintptr_t ds; \
    struct general_purpose_registers general_regs; \
    uint32_t int_no; \
    uint32_t error_code; \
    /* pushed automatically */ \
    uint32_t eip; \
    uint32_t cs; \
    uint32_t eflags;
// clang-format on

/*
interrupt_frame_kernel and interrupt_frame are compatible so we will use
struct interrupt_frame as generic type and access esp and ss carefully
*/

struct interrupt_frame {
    INTERRUPT_FRAME_COMMON
    /* pushed automatically when switching from user to kernel */
    uint32_t esp;
    uint32_t ss;
} __attribute__((packed));

struct interrupt_frame_kernel {
    INTERRUPT_FRAME_COMMON
} __attribute__((packed));

#endif
