#include "memcheck.h"
#include "kheap.h"
#include "../config.h"
#include "../string/string.h"

struct memcheck_entry *allocation_table;

void memcheck_table_init(int size)
{
    allocation_table = kzalloc(size * sizeof(struct memcheck_entry));
}

void memcheck_allocate(void *ptr, size_t size, const char * const filename, const char * const function, int line)
{
    const size_t block = kheap_addr_to_block(ptr);
    allocation_table[block].blocks = (size + KERNEL_HEAP_BLOCK_SIZE - 1) / KERNEL_HEAP_BLOCK_SIZE;
    allocation_table[block].line = line;
    strncpy(allocation_table[block].filename, filename, sizeof(allocation_table[block].filename)); //NOLINT
    strncpy(allocation_table[block].function, function, sizeof(allocation_table[block].filename)); //NOLINT
}

void memcheck_free(void *ptr)
{
    const size_t block = kheap_addr_to_block(ptr);
    memset(&allocation_table[block], 0, sizeof(struct memcheck_entry)); //NOLINT
    allocation_table[block].line = -1;
}