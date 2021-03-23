#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <stdint.h>
#include "task.h"
#include "../../config.h"

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

#define next_process(t) ((t)->next)

#define for_each_process(t) for (t = next_process(&idle_task); t != &idle_process; t = next_process(t))

void task_init();

union task *task_new();
void task_free(union task *task);
void print_tasks();
int process_from_function(void *f, char *name, struct process **process);
void init_idle_process();
int process_load(const char *filename);
int process_init(const char *name, void *memory, uint32_t memory_size, int privileged, int slot, struct process **process);
void process_switch_directory(struct process *process);
#endif
