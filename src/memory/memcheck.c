#include "memcheck.h"
#include "kheap.h"
#include "../config.h"
#include "../string/string.h"
#include "../kernel/panic.h"
#include "../termio/termio.h"

struct memcheck_entry *allocation_table;
size_t allocation_table_len;

void memcheck_table_init(int size)
{
    allocation_table_len = size;
    allocation_table = kzalloc(size * sizeof(struct memcheck_entry));
}

void memcheck_allocate(void *ptr, size_t size, const char *const filename, const char *const function, int line)
{
    const size_t block = kheap_addr_to_block(ptr);
    allocation_table[block].blocks = (size + KERNEL_HEAP_BLOCK_SIZE - 1) / KERNEL_HEAP_BLOCK_SIZE;
    allocation_table[block].line = line;
    allocation_table[block].ptr = (uintptr_t)ptr;
    strncpy(allocation_table[block].filename, filename, sizeof(allocation_table[block].filename)); //NOLINT
    strncpy(allocation_table[block].function, function, sizeof(allocation_table[block].filename)); //NOLINT
}

void memcheck_free(void *ptr)
{
    const size_t block = kheap_addr_to_block(ptr);

    if (allocation_table[block].blocks == 0) {
        panic("DOUBLE FREE");
    }

    memset(&allocation_table[block], 0, sizeof(struct memcheck_entry)); //NOLINT
    allocation_table[block].line = -1;
}

void memcheck_check(char *skip)
{
    print_char('\n');
    for (size_t i = 0; i < allocation_table_len; i++) {
        if (allocation_table[i].blocks == 0) {
            continue;
        }

        if (!strcmp(allocation_table[i].function, skip)) {
            continue;
        }

        print("alloc: ");
        print(allocation_table[i].filename);
        print_char(':');
        print(allocation_table[i].function);
        print_char(':');
        char buffer[100];
        itoa(allocation_table[i].line, buffer);
        print(buffer);
        print(" addr: ");
        itoa(allocation_table[i].ptr, buffer);
        print(buffer);
        print_char('\n');
    }
}