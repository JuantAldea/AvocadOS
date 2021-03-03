#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <stdint.h>
#include "task.h"
#include "../../config.h"

struct process {
    uint16_t pid;
    char filename[MAX_PATH_LEN];
    struct task *task;
    void *allocations[4096];
    void *memory;
    uint32_t memory_size;
    void *stack;
};

void task_init();

struct task *task_new();
void task_free(struct task *task);
void print_tasks();

#endif
