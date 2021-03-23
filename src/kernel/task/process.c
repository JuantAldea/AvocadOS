#include "process.h"
#include "../../config.h"
#include "../../status.h"
#include "../../string/string.h"
#include "../../memory/kheap.h"
#include "../../kernel/kernel.h"
#include "../../fs/file.h"
#include "../../kernel/panic.h"
#include "../../memory/paging.h"

#include "../../termio/termio.h"

struct process *idle_process;

void idle_function()
{
    while (1) {
        print("IDLE TASK\n");
        asm volatile("hlt");
    }
}

struct process *current_process = NULL;
static struct process *processes[MAX_PROCESSES] = { 0 };

struct process *process_current()
{
    return current_process;
}

struct process *process_next()
{
    return current_process->next;
}

struct process *process_previous()
{
    return current_process->previous;
}

struct process *process_list_insert(struct process *process)
{
    process->previous = current_process->previous;
    process->next = current_process;

    current_process->previous = process;
    process->previous->next = process;

    return process;
}

struct process *process_list_remove(struct process *process)
{
    process->previous->next = process->next;
    process->next->previous = process->previous;
    process->next = NULL;
    process->previous = NULL;
    return process;
}

struct process *process_get(int pid)
{
    if (pid < 0 || pid >= MAX_PROCESSES) {
        return ERROR(-EINVAL);
    }

    return processes[pid];
}

static int process_load_binary(const char *filename, void **memory, uint32_t *len)
{
    int res = 0;
    *memory = NULL;

    struct FILE *fd = fopen(filename, "r");

    if (!fd) {
        res = -EIO;
        goto out;
    }

    struct stat file_stat;
    res = fstat(fd->fileno, &file_stat);

    if (res) {
        goto out;
    }

    *memory = kzalloc(file_stat.st_size);

    if (!*memory) {
        res = -ENOMEM;
        goto out;
    }

    size_t read_bytes = fread(*memory, file_stat.st_size, 1, fd);

    if (read_bytes != file_stat.st_size) {
        res = -EIO;
        goto out;
    }

    *len = file_stat.st_size;

out:
    fclose(fd);

    if (res) {
        if (fd) {
            fclose(fd);
        }

        if (*memory) {
            kfree(*memory);
            *memory = NULL;
        }

        *len = 0;
    }

    return res;
}

int process_map_memory(struct process *process)
{
    if (!process->memory) {
        panic("PROCESS MEMORY NOT LOADED");
    }

    if (!process) {
        panic("PROCESS BROKEN");
    }

    if (!process->task) {
        panic("NO PROCESS TASK");
    }

    // map code
    int res = paging_map_from_to(process->page_directory, (void *)PROGRAM_BASE_VIRT_ADDR, process->memory,
                                 paging_align_address(process->memory + process->memory_size),
                                 PAGING_WRITABLE_PAGE | PAGING_ACCESS_FROM_ALL | PAGING_PRESENT);
    if (res) {
        goto out;
    }

    // map stack
    res = paging_map_from_to(process->page_directory, (void *)PROGRAM_STACK_END, process->stack,
                             paging_align_address(process->stack + PROGRAM_STACK_SIZE),
                             PAGING_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_WRITABLE_PAGE);
    if (res) {
        goto out;
    }

    if (res) {
        goto out;
    }
out:
    return res;
}

void process_init_process_stack(struct process *process)
{
    struct isr_data isr_data;
    //task_fill_isr_frame(process->task, &isr_data);
    memcpy(&(((uint8_t *)(process->stack))[PROGRAM_STACK_SIZE - 1]) - sizeof(isr_data), &isr_data, sizeof(isr_data)); //NOLINT
}

void init_idle_process()
{
    void *memory;
    uint32_t memory_size;
    process_load_binary("0:/TRAP.BIN", &memory, &memory_size);
    process_init("Idle Process", memory, memory_size, 0, 0, &processes[0]);
    processes[0]->stack = kzalloc(PROGRAM_STACK_SIZE);
    processes[0]->task = task_init_idle_task(processes[0]);
    process_map_memory(processes[0]);
    process_init_process_stack(processes[0]);

    processes[0]->next = processes[0];
    processes[0]->previous = processes[0];
    current_process = processes[0];
}

int process_init(const char *name, void *memory, uint32_t memory_size, int privileged, int slot, struct process **process)
{
    int res = 0;

    *process = kzalloc(sizeof(struct process));
    struct process *_process = *process;

    if (!_process) {
        return -ENOMEM;
    }

    _process->page_directory = paging_init_4gb_directory(PAGING_PRESENT | PAGING_ACCESS_FROM_ALL);

    if (!_process->page_directory) {
        res = -ENOMEM;
        goto out;
    }

    _process->pid = slot;
    _process->memory_size = memory_size;
    _process->memory = memory;

    _process->stack = kzalloc(PROGRAM_STACK_SIZE);

    if (!_process->stack) {
        res = -EINVAL;
        goto out;
    }

    strncpy(_process->name, name, sizeof(_process->name)); //NOLINT

    _process->task = task_new(_process, privileged);

    if (!_process->task) {
        res = -ENOMEM;
        goto out;
    }

    // handle error in task

    res = process_map_memory(_process);
    if (res) {
        goto out;
    }

out:
    if (IS_ERROR(res)) {
        if (_process) {
            if (_process->stack) {
                kfree(_process->stack);
            }

            if (_process->memory) {
                kfree(_process->memory);
            }

            if (_process->page_directory) {
                paging_free_4gb_directory(_process->page_directory);
            }

            if (_process->task) {
                task_free(_process->task);
            }

            kfree(_process);
        }
    }

    return res;
}

int process_load(const char *filename)
{
    int slot_found = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (!process_get(i)) {
            slot_found = i;
            break;
        }
    }

    if (slot_found < 0) {
        return -ENOMEM;
    }

    void *memory;
    uint32_t memory_size;
    process_load_binary(filename, &memory, &memory_size);
    return process_init("Idle Process", memory, memory_size, 1, slot_found, &processes[slot_found]);
}

void process_switch_directory(struct process *process)
{
    paging_switch_directory(process->page_directory);
}