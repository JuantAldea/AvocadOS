#include "task.h"
#include "../../string/string.h"
#include "../../memory/paging.h"
#include "../../memory/kheap.h"
#include "../../status.h"
#include "../../config.h"
#include "../../kernel/panic.h"

#define PREVIOUS_TASK(task_ptr_ptr) *(task_ptr_ptr - offsetof(struct task, next))

struct task *task_head;
struct task *current_task;

int __task_init(struct task *task);

int counter = 0;

struct task *task_current()
{
    return current_task;
}

struct task *task_next()
{
    return current_task->next;
}

struct task *task_previous()
{
    return current_task->previous;
}

struct task *task_list_insert(struct task *task)
{
    task->previous = task_head->previous;
    task->next = task_head;

    task_head->previous = task;
    task->previous->next = task;

    return task;
}

struct task *task_list_remove(struct task *task)
{
    task->previous->next = task->next;
    task->next->previous = task->previous;
    task->next = NULL;
    task->previous = NULL;
    return task;
}

struct task *task_new()
{
    int res = 0;
    struct task *task = kzalloc(sizeof(*task));
    if (!task) {
        res = -ENOMEM;
        goto out;
    }

    __task_init(task);
    task_list_insert(task);
out:
    return res ? task : NULL;
}

void task_free(struct task *task)
{
    paging_free_4gb_directory(task->page_directory);
}

#include "../../termio/termio.h"

void print_tasks()
{
    struct task *ptr = task_head;

    do {
        char buffer[100];
        itoa(ptr->id, buffer);
        print(buffer);
        print(", ");
        ptr = ptr->next;
    } while (ptr != task_head);

    print("\n");
}

void task_init()
{
    task_head = kzalloc(sizeof(*task_head));
    if (!task_head) {
        panic("TASK INIT");
    }

    __task_init(task_head);

    task_head->next = task_head;
    task_head->previous = task_head;
    current_task = task_head;
}

int __task_init(struct task *task)
{
    memset(task, 0, sizeof(*task)); //NOLINT

    task->page_directory = paging_init_4gb_directory(PAGING_PRESENT | PAGING_ACCESS_FROM_ALL);
    if (!task->page_directory) {
        return -ENOMEM;
    }

    task->id = counter++;
    task->registers = (struct registers){ .eip = PROGRAM_BASE_VIRT_ADDR, .ss = USER_DATA_SEGMENT, .esp = PROGRAM_STACK_BASE_VIRT_ADDR };

    return 0;
}
