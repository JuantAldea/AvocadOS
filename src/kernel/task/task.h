#ifndef __TASK_H__
#define __TASK_H__

#include "../registers.h"
#include "../../memory/paging.h"

struct task {
    struct page_directory_handle *page_directory;
    struct registers registers;

    struct task *next;
    struct task *previous;
    int id;
};

void task_init();

struct task *task_new();
void task_free(struct task *task);
void print_tasks();

#endif
