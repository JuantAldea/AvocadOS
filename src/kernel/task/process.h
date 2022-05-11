#ifndef __PROCESS_H__
#define __PROCESS_H__
#if 0
#include <stdint.h>
#include "../../config.h"
#include "../../idt/idt.h"

struct task_control_block {
    struct process_state registers;
    struct process *process;
    uintptr_t kernel_stack_pointer;
    union task *next;
    union task *previous;
};

union task {
    struct task_control_block task;
    uint32_t kstack[TASK_KERNEL_STACK_SIZE];
};

struct process {
    int pid;
    char name[MAX_PATH_LEN];
    union task *task;
    struct page_directory_handle *page_directory;
    void *memory;
    uint32_t memory_size;
    void *stack;
    struct process *next;
    struct process *previous;
};

extern struct process *current_process;
#define next_process(t) ((t)->next)

#define for_each_process(t) for (t = next_process(&idle_task); t != &idle_process; t = next_process(t))

void tasking_init();

void task_free(union task *task);
void print_tasks();
int process_from_function(void *f, char *name, struct process **process);

int process_load(const char *filename);
int process_init(const char *name, void *memory, uint32_t memory_size, int privileged, int slot, struct process **process);

#endif
#endif