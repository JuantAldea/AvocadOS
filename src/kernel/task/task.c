#if 0
#include "task.h"
#include "../../string/string.h"
#include "../../memory/paging.h"
#include "../../memory/kheap.h"
#include "../../status.h"
#include "../../config.h"
#include "../../kernel/panic.h"

#define PREVIOUS_TASK(task_ptr_ptr) *(task_ptr_ptr - offsetof(struct task, next))
extern void process_switch_directory(struct process *process);

union task idle_task;
static union task *current_task;

static int __task_init(union task *task, struct process *process, int privileged);

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

union task *task_new(struct process *process, int privileged)
{
    int res = 0;
    union task *task = kzalloc(sizeof(*task));
    if (!task) {
        res = -ENOMEM;
        goto out;
    }

    __task_init(task, process, privileged);
    task_list_insert(task);
out:
    return res ? task : NULL;
}

void task_free(union task *task)
{
    (void)task;
}

union task *task_init_idle_task(struct process *process)
{
    __task_init(&idle_task, process, 0);
    idle_task.task.next = &idle_task;
    idle_task.task.previous = &idle_task;
    current_task = &idle_task;
    return &idle_task;
}

static int __task_init(union task *task, struct process *process, int privileged)
{
    memset(task, 0, sizeof(*task)); //NOLINT


    struct isr_data *frame = (void *)(task->kstack - sizeof(struct isr_data) - 1);

    frame->int_no = 0;
    frame->err_code = 0;
    frame->regs = (struct general_purpose_registers){ 0 };

    uint32_t cs = privileged ? GDT_KERNEL_CODE_SEGMENT_SELECTOR : GDT_USER_CODE_SEGMENT_SELECTOR | 0x3;
    uint32_t ds = privileged ? GDT_KERNEL_DATA_SEGMENT_SELECTOR : GDT_USER_DATA_SEGMENT_SELECTOR | 0x3;

    frame->gs = ds;
    frame->ds = ds;
    frame->fs = ds;
    frame->es = ds;

    frame->isr_frame = (struct isr_frame){
        .ss = ds,
        .cs = cs,
        .eip = PROGRAM_BASE_VIRT_ADDR,
        .esp = PROGRAM_STACK_BASE_VIRT_ADDR,
        .eflags = 0x200,
    };

    task->task.process = process;
    return 0;
}

/*
void task_save_current_task(struct isr_data *isr_data)
{
    current_task->registers = (struct process_state){
        .eflags = isr_data->isr_frame.eflags,
        .eip = isr_data->isr_frame.eip,
        .segments =
                (struct segment_registers){
                        .cs = isr_data->isr_frame.cs,
                        .ss = isr_data->isr_frame.ss,
                        .gs = isr_data->gs,
                        .fs = isr_data->fs,
                        .es = isr_data->es,
                        .ds = isr_data->ds,

                },
        .regs =
                (struct general_purpose_registers){
                        .esp = isr_data->isr_frame.esp,
                        .esi = isr_data->regs.esi,
                        .ebp = isr_data->regs.ebp,
                        .ebx = isr_data->regs.ebx,
                        .edx = isr_data->regs.edx,
                        .ecx = isr_data->regs.ecx,
                        .eax = isr_data->regs.eax,
                },
    };
}
*/

void task_store(struct isr_data *isr_data)
{
    current_task->task.esp0 = isr_data;
}

#include "tss.h"
#include "../gdt.h"

void task_restore()
{
    //process_switch_directory(current_task->process);
    tss.esp0 = (uintptr_t)current_task->task.esp0;
    tss_load(sizeof(struct gdt_native) * 5);
    //task_continue((struct isr_data *)current_task->esp0);
}

void enter_user()
{
    extern void load_user_segments();
    load_user_segments();
    process_switch_directory(current_task->task.process);
}
/*
void task_place_isr_frame_on_stack(union task *task, struct isr_data *isr_data)
{
    (*task->process->) = (struct isr_data){
        .gs = task->registers.segments.gs,
        .fs = task->registers.segments.fs,
        .es = task->registers.segments.es,
        .ds = task->registers.segments.ds,
        .regs =
                (struct general_purpose_registers){
                        .eax = task->registers.regs.eax,
                        .ebp = task->registers.regs.ebp,
                        .ebx = task->registers.regs.ebx,
                        .ecx = task->registers.regs.ecx,
                        .edi = task->registers.regs.edi,
                        .edx = task->registers.regs.edx,
                        .esi = task->registers.regs.edi,
                        .esp = task->registers.regs.esp,
                },
        .int_no = 0,
        .err_code = 0,
        .isr_frame =
                (struct isr_frame){
                        .cs = task->registers.segments.cs,
                        .eflags = task->registers.eflags,
                        .eip = task->registers.eip,
                        .esp = task->registers.regs.esp,
                        .ss = task->registers.segments.ss,
                },
    };
}
*/
void schedule(void)
{
    union task *next = task_next();
    if (next != current_task) {
        //switch_task(next);
    }
}
#endif