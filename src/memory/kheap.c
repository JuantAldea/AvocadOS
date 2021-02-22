#include "kheap.h"
#include "heap.h"
#include "../config.h"
#include "../termio/termio.h"
#include "../string/string.h"
#include "memcheck.h"

struct heap kernel_heap;
struct heap_table kheap_table;

void kheap_init()
{
    kheap_table.entries = (HEAP_TABLE_ENTRY *)KERNEL_HEAP_TABLE_ADDRESS;
    kheap_table.len = KERNEL_HEAP_SIZE / KERNEL_HEAP_BLOCK_SIZE;
    void *end = (void *)(KERNEL_HEAP_ADDRESS + KERNEL_HEAP_SIZE);
    if (heap_create(&kernel_heap, (void *)KERNEL_HEAP_ADDRESS, end, &kheap_table)) {
        print("Error creating kheap\n");
    //PANIC
    trap:
        goto trap;
    }
#ifdef MEMCHECK
    memcheck_table_init(kheap_table.len);
#endif
}

void *kmalloc(size_t size)
{
    return heap_malloc(&kernel_heap, size);
}

void *__kzalloc(size_t size)
{
    void *ptr = heap_malloc(&kernel_heap, size);

    if (!ptr) {
        return NULL;
    }

    memset(ptr, 0, size); // NOLINT
    return ptr;
}

void __kfree(void *ptr)
{
    heap_free(&kernel_heap, ptr);
}

size_t kheap_count_used_blocks()
{
    return count_used_blocks(&kernel_heap);
}

size_t kheap_addr_to_block(void *ptr)
{
    return heap_addr_to_block(&kernel_heap, ptr);
}

void *__memcheck_kzalloc(size_t size, const char *filename, const char *function, int line)
{
    void *ptr = __kzalloc(size);
    memcheck_allocate(ptr, size, filename, function, line);
    return ptr;
}

void __memcheck_kfree(void *ptr)
{
    memcheck_free(ptr);
    __kfree(ptr);
}
