#ifndef __TASK_H__
#define __TASK_H__

#include "../../cpu.h"
#include "../../idt/idt.h"
#include "../../memory/paging.h"
#include "../../config.h"
#include "process.h"

#define next_task(t) ((t)->next)

#define for_each_task(t) for (t = next_task(&idle_task); t != &idle_task; t = next_task(t))

union task *task_init_idle_task(struct process *process);

extern union task *current_task;

union task *task_new(struct process *process, int privileged, uintptr_t entry_point, uintptr_t esp);
void task_free(union task *task);
void print_tasks();
//int switch_task(union task *task);
void task_schedule_next();

int task_switch_to_task_page_directory();
extern void task_continue(struct interrupt_frame *interrupt_frame);
extern void restore_general_puporse_registers(struct interrupt_frame *interrupt_frame);
void enter_user();

void task_save_current_task(struct interrupt_frame *interrupt_frame);

uintptr_t schedule(struct interrupt_frame *interrupt_frame);
void task_store(union task *task, struct interrupt_frame *interrupt_frame);
void task_push_data_to_kstack(union task *task, void *data, size_t size);
void task_push_state_to_kstack(union task *task);

#endif
