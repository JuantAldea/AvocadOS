#include "process.h"
#include "../../config.h"
#include "../../status.h"
#include "../../string/string.h"
#include "../../memory/kheap.h"
#include "../../kernel/kernel.h"
#include "../../fs/file.h"
#include "../../kernel/panic.h"
#include "../../memory/paging.h"

struct process *current_process = NULL;
static struct process *processes[MAX_PROCESSES] = { 0 };

static void process_init(struct process *process)
{
    memset(process, 0, sizeof(*process)); //NOLINT
}

struct process *process_get(int pid)
{
    if (pid < 0 || pid >= MAX_PROCESSES) {
        return ERROR(-EINVAL);
    }

    return processes[pid];
}

static int process_load_binary(const char *filename, struct process *process)
{
    int res = 0;

    int fd = fopen(filename, "r");

    if (fd < 0) {
        res = fd;
        goto out;
    }

    struct stat file_stat;
    res = fstat(fd, &file_stat);

    if (res) {
        goto out;
    }

    process->memory = kzalloc(file_stat.st_size);

    if (!process->memory) {
        res = -ENOMEM;
        goto out;
    }

    size_t read_bytes = fread(fd, process->memory, file_stat.st_size, 1);

    if (read_bytes != file_stat.st_size) {
        res = -EIO;
        goto out;
    }

    process->memory_size = file_stat.st_size;

out:
    fclose(fd);

    if (res) {
        if (fd >= 0) {
            fclose(fd);
        }

        if (process->memory) {
            kfree(process->memory);
        }
    }

    return res;
}

static int process_load_data(const char *filename, struct process *process)
{
    return process_load_binary(filename, process);
}

int process_map_binary(struct process *process)
{
    if (!process || !process->task) {
        panic("PROCESS BROKEN");
    }

    return paging_map_from_to(process->task->page_directory, (void *)PROGRAM_BASE_VIRT_ADDR, process->memory,
                              paging_align_address(process->memory + process->memory_size),
                              PAGING_WRITABLE_PAGE | PAGING_ACCESS_FROM_ALL | PAGING_PRESENT);
}

int process_map_memory(struct process *process)
{
    if (!process->memory) {
        panic("PROCESS MEMORY NOT LOADED");
    }

    return process_map_binary(process);
}

int process_load_for_slot(const char *filename, struct process **process, int slot)
{
    if (process_get(slot)) {
        return -EINVAL;
    }

    struct process *_process = kzalloc(sizeof(*_process));

    if (!_process) {
        return -ENOMEM;
    }

    process_init(_process);

    _process->pid = slot;

    int res = process_load_data(filename, _process);
    if (res) {
        goto out;
    }

    _process->stack = kzalloc(PROGRAM_STACK_SIZE);

    if (!_process->stack) {
        res = -EINVAL;
        goto out;
    }

    strncpy(_process->filename, filename, sizeof(_process->filename)); //NOLINT

    _process->task = task_new(_process);

    // handle error in task

    res = process_map_memory(_process);
    if (res) {
        goto out;
    }

    *process = _process;
    processes[slot] = _process;
out:
    if (IS_ERROR(res)) {
        if (_process) {
            if (_process->stack) {
                kfree(_process->stack);
            }

            if (_process->memory) {
                kfree(_process->memory);
            }

            if (_process->task) {
                task_free(_process->task);
            }

            kfree(_process);
        }
    }

    return res;
}

int process_load(const char *filename, struct process **process)
{
    int slout_found = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (!process_get(i)) {
            slout_found = i;
            break;
        }
    }

    if (slout_found < 0) {
        return -ENOMEM;
    }

    return process_load_for_slot(filename, process, slout_found);
}