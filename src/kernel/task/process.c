#if 0
#include "process.h"
#include "task.h"
#include "../../config.h"
#include "../../status.h"
#include "../../string/string.h"
#include "../../memory/kheap.h"
#include "../../kernel/kernel.h"
#include "../../fs/file.h"
#include "../../kernel/panic.h"
#include "../../memory/paging.h"
#include "../../termio/termio.h"

struct process *current_process = NULL;
static struct process *processes[MAX_PROCESSES] = { 0 };
extern union task system_task;

void idle_function()
{
    while (1) {
        /*
        for (int i = 0; i < 5; i++) {
            print("Idle - ");
            char buffer[100];
            itoa(i, buffer);
            print(buffer);
            print("                         \n");
        }
        asm volatile("hlt");
        */
        asm volatile("hlt");
    }
}

struct process *process_current()
{
    return current_process;
}

struct process *process_list_insert(struct process *process)
{
    struct process *system_process = system_task.task.process;
    process->next = system_process;
    process->previous = system_process->previous;
    system_process->previous = process;
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

    process->page_directory = paging_clone_directory(&kernel_page_directory);

    // map code
    uintptr_t physical_code_pointer_begin = virtual_to_physical_addr(current_directory, process->memory);
    uintptr_t physical_code_pointer_end = paging_align_address(physical_code_pointer_begin + process->memory_size);

    int res = paging_map_from_to(process->page_directory, PROGRAM_BASE_VIRT_ADDR, physical_code_pointer_begin, physical_code_pointer_end,
                                 PAGING_WRITABLE_PAGE | PAGING_ACCESS_FROM_ALL | PAGING_PRESENT);
    if (res) {
        goto out;
    }

    // map stack
    uintptr_t physical_stack_pointer_begin = virtual_to_physical_addr(current_directory, process->stack);
    uintptr_t physical_stack_pointer_end = paging_align_address(physical_stack_pointer_begin + PROGRAM_STACK_SIZE);

    res = paging_map_from_to(process->page_directory, PROGRAM_STACK_TOP_VIRT_ADDR, physical_stack_pointer_begin, physical_stack_pointer_end,
                             PAGING_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_WRITABLE_PAGE);
    if (res) {
        goto out;
    }

out:
    return res;
}

void init_system_process()
{
    /*
        The first step of interruption handling is saving the processor state into current_task's kernel stack.
        We will get leverage from that by setting the kernel stack, defined at kernel.asm, as the stack of the
        current_task, by doing so, after triggering an interrupt, the processor will save its state into the
        aforementioned stack
    */
    struct process *process = kzalloc(sizeof(struct process));
    process->pid = 0;
    process->memory_size = 0;
    process->memory = (void *)KERNEL_VIRTUAL_BASE;
    strncpy(process->name, "System Process", sizeof(process->name)); //NOLINT
    process->page_directory = &kernel_page_directory;
    process->task = &system_task;
    process->next = process;
    process->previous = process;

    // cs won't be stored by the first schedule() as there won't be a ring change
    system_task.task.registers.segments.ss = GDT_KERNEL_DATA_SEGMENT_SELECTOR;
    system_task.task.process = process;

    system_task.task.next = &system_task;
    system_task.task.previous = &system_task;

    current_process = process;
    current_task = process->task;

    processes[process->pid] = process;

    raise_int_0x20();
}

void init_idle_process()
{
    struct process *process = kzalloc(sizeof(struct process));

    process->pid = 1;
    process->memory_size = 0;
    process->memory = (void *)KERNEL_VIRTUAL_BASE;

    strncpy(process->name, "Idle Process", sizeof(process->name)); //NOLINT
    process->page_directory = &kernel_page_directory;
    process->stack = kzalloc(PROGRAM_STACK_SIZE);

    process->task = task_new(process, 1, (uintptr_t)&idle_function, (uintptr_t)(((char *)process->stack)[PROGRAM_STACK_SIZE]));

    processes[process->pid] = process;
    process_list_insert(process);
}

void tasking_init()
{
    init_system_process();
    init_idle_process();
    process_load("0:/TRAP.BIN");
}

int process_init(const char *name, void *memory, uint32_t memory_size, int privileged, int slot, struct process **process)
{
    int res = 0;

    *process = kzalloc(sizeof(struct process));
    struct process *_process = *process;

    if (!_process) {
        return -ENOMEM;
    }

    _process->pid = slot;
    _process->memory_size = memory_size;
    _process->memory = memory;

    if (!(_process->stack = kzalloc(PROGRAM_STACK_SIZE))) {
        res = -EINVAL;
        goto out;
    }

    if ((res = process_map_memory(_process))) {
        goto out;
    }

    strncpy(_process->name, name, sizeof(_process->name)); //NOLINT

    _process->task = task_new(_process, privileged, PROGRAM_BASE_VIRT_ADDR, PROGRAM_STACK_TOP_VIRT_ADDR + PROGRAM_STACK_SIZE);

    if (!_process->task) {
        res = -ENOMEM;
        goto out;
    }

    // handle error in task
    if (res) {
        goto out;
    }

    process_list_insert(_process);

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
                //paging_free_4gb_directory(_process->page_directory);
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
    return process_init(filename, memory, memory_size, 0, slot_found, &processes[slot_found]);
}

/*
void process_switch_directory(struct process *process)
{
    paging_switch_directory(process->page_directory);
}
*/

#endif