#include "task.h"
#include "../../string/string.h"
#include "../../memory/paging.h"
#include "../../memory/kheap.h"
#include "../../status.h"
#include "../../config.h"
#include "../../kernel/panic.h"
#include "../gdt.h"
#include "../../termio/termio.h"

#define PREVIOUS_TASK(task_ptr_ptr) *(task_ptr_ptr - offsetof(struct task, next))

//extern void process_switch_directory(struct process *process);

union task *current_task;

//static int _task_init(union task *task, struct process *process, int privileged);

union task *task_current()
{
    return current_task;
}

union task *task_next()
{
    return current_task->task.next;
}

union task *task_previous()
{
    return current_task->task.previous;
}

union task *task_list_insert(union task *task)
{
    task->task.previous = current_task->task.previous;
    task->task.next = current_task;
    current_task->task.previous = task;
    task->task.previous->task.next = task;

    return task;
}

union task *task_list_remove(union task *task)
{
    task->task.previous->task.next = task->task.next;
    task->task.next->task.previous = task->task.previous;
    task->task.next = NULL;
    task->task.previous = NULL;
    return task;
}

union task *task_new(struct process *process, int privileged, uintptr_t entry_point, uintptr_t esp)
{
    int res = 0;
    union task *task = kzalloc(sizeof(*task));
    if (!task) {
        res = -ENOMEM;
        goto out;
    }

    const uintptr_t cs = privileged ? GDT_KERNEL_CODE_SEGMENT_SELECTOR : (GDT_USER_CODE_SEGMENT_SELECTOR | 0x3);
    const uintptr_t ss = privileged ? GDT_KERNEL_DATA_SEGMENT_SELECTOR : (GDT_USER_DATA_SEGMENT_SELECTOR | 0x3);

    *task = (union task) {
        .task = {
            .registers.eip = entry_point,
            .registers.general_regs = {
                .edi = 0,
                .esi = 0,
                .ebp = 0,
                .esp = esp,
                .ebx = 0,
                .edx = 0,
                .ecx = 0,
                .eax = 0xDEADC0DE,
            },
            .registers.eflags = 0x202,
            .registers.segments = {
                .gs = ss,
                .fs = ss,
                .es = ss,
                .ds = ss,
                .ss = ss,
                .cs = cs,
            },
            .kernel_stack_pointer = (uintptr_t)&task->kstack[TASK_KERNEL_STACK_SIZE],
            .next = NULL,
            .previous = NULL,
            .process = process,
        },
    };

    task_push_state_to_kstack(task);

    task_list_insert(task);
out:
    return !res ? task : NULL;
}

void task_free(union task *task)
{
    (void)task;
}

void task_push_data_to_kstack(union task *task, void *data, size_t size)
{
    task->task.kernel_stack_pointer -= size;
    memcpy((void *)task->task.kernel_stack_pointer, data, size); //NOLINT
}

void task_push_state_to_kstack(union task *task)
{
    const int user_mode = task->task.registers.segments.cs & 0x3;
    const size_t interrupt_frame_size = user_mode ? sizeof(struct interrupt_frame) : sizeof(struct interrupt_frame_kernel);

    struct interrupt_frame interrupt_frame = {
        .gs = task->task.registers.segments.gs,
        .fs = task->task.registers.segments.fs,
        .es = task->task.registers.segments.es,
        .ds = task->task.registers.segments.ds,
        .general_regs = task->task.registers.general_regs,
        .int_no = 0xDEADC0DE,
        .error_code = 0xDEADC0DE,
        .eip = task->task.registers.eip,
        .cs = task->task.registers.segments.cs,
        .eflags = task->task.registers.eflags,
        // won't be pushed if there's no context switch
        .esp = task->task.registers.general_regs.esp,
        .ss = task->task.registers.segments.ss,
    };

    task_push_data_to_kstack(task, &interrupt_frame, interrupt_frame_size);
}

void switch_task(union task *previous, union task *next)
{
    (void)next;
    (void)previous;
    //asm volatile("cli");

    asm volatile("mov %0, %%cr3" ::"r"(PAGE_ENTRY_ADDR(next->task.process->page_directory->directory[1023])));

    uintptr_t page_directory_addr;
    asm volatile("mov %%cr3, %0" : "=r"(page_directory_addr));
    asm volatile("mov %0, %%cr3" : : "r"(page_directory_addr));

    tss_set_kernel_stack((uintptr_t)&next->kstack[TASK_KERNEL_STACK_SIZE]);
    tss_load(GDT_TSS_SEGMENT_SELECTOR);
    current_process = next->task.process;
    current_task = next;
}

void task_store(union task *task, struct interrupt_frame *interrupt_frame)
{
    if (!current_task) {
        panic("NO CURRENT TASK!");
    }

    task->task.kernel_stack_pointer = (uintptr_t)interrupt_frame;

    if (interrupt_frame->cs & 0x3) {
        //context change
        task->task.registers.segments.ss = interrupt_frame->ss;
        task->task.registers.general_regs.esp = interrupt_frame->esp;
    }

    task->task.registers.eflags = interrupt_frame->eflags;
    task->task.registers.general_regs = interrupt_frame->general_regs;
    task->task.registers.eip = interrupt_frame->eip;
    task->task.registers.segments.cs = interrupt_frame->cs;
    task->task.registers.segments.gs = interrupt_frame->gs;
    task->task.registers.segments.fs = interrupt_frame->fs;
    task->task.registers.segments.es = interrupt_frame->es;
    task->task.registers.segments.ds = interrupt_frame->ds;
}

uintptr_t schedule(struct interrupt_frame *interrupt_frame)
{
    if (!current_task) {
        return (uintptr_t)interrupt_frame;
    }

    task_store(current_task, interrupt_frame);

    union task *next = task_next();

    /*
    if (next == current_task) {
        return;
    }
    */
    print(current_task->task.process->name);
    print(" -> ");
    print(next->task.process->name);
    print_char('\n');

    switch_task(current_task, next);

    return current_task->task.kernel_stack_pointer;
}

/*
break task.c:198
commands
silent
printf "%x, %x\n", current_task, next
p/x current_task->task
p/x next->task
cont
end
*/
