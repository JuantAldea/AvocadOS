#ifndef __TASK_H__
#define __TASK_H__

#include "../../cpu.h"
#include "../../idt/idt.h"
#include "../../memory/paging.h"

struct process;
struct task_control_block {
    //struct process_state registers;
    struct process *process;
    void *esp0;
    union task *next;
    union task *previous;
};

union task {
    struct task_control_block task;
    uint32_t kstack[PAGING_PAGE_SIZE / sizeof(uint32_t)];
};

#define next_task(t) ((t)->next)

#define for_each_task(t) for (t = next_task(&idle_task); t != &idle_task; t = next_task(t))

union task *task_init_idle_task(struct process *process);

union task *task_new(struct process *process, int privileged);
void task_free(union task *task);
void print_tasks();
//int switch_task(union task *task);
void task_schedule_next();

int task_switch_to_task_page_directory();
extern void task_continue(struct isr_data *isr_data);
extern void restore_general_puporse_registers(struct isr_data *isr_data);
void enter_user();

void task_save_current_task(struct isr_data *isr_data);

void schedule(void);
void task_store(struct isr_data *isr_data);

#endif
