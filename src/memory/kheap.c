#include "kheap.h"
#include "heap.h"
#include "../config.h"
#include "../termio/termio.h"
#include "../string/string.h"
#include "memcheck.h"
#include "../kernel/panic.h"

struct heap kernel_heap;
struct heap_table kheap_table;

/*
uintptr_t early_kmalloc_current_pos;
void *early_kzalloc(size_t len)
{
    void *ptr = (void *)early_kmalloc_current_pos;

    if (!paging_addr_is_page_aligned(ptr)) {
        panic("early_kmalloc: paging_addr_is_page_aligned: false ");
    }

    len = PAGING_PAGE_SIZE * ((len + PAGING_PAGE_SIZE - 1) / PAGING_PAGE_SIZE);

    early_kmalloc_current_pos += len;

    memset(ptr, 0, len); //NOLINT

    bitmap_begin[PAGE_BITMAP_INDEX(bitmap_begin)] = 1;

    return ptr;
}
*/

void kheap_init(uintptr_t addr)
{
    addr = (addr & 0xFFF) ? (addr & ~0xFFF) + 0x1000 : addr;
    kheap_table.entries = (HEAP_TABLE_ENTRY *)addr;
    kheap_table.len = KERNEL_HEAP_SIZE / KERNEL_HEAP_BLOCK_SIZE;
    void *end = (void *)(addr + KERNEL_HEAP_SIZE);
    if (heap_create(&kernel_heap, (void *)addr, end, &kheap_table)) {
        panic("Error creating kheap\n");
    }

#ifdef MEMCHECK
    memcheck_table_init(kheap_table.len);
    memcheck_allocate((void *)addr, kheap_table.len, __FILE__, __FUNCTION__, __LINE__);
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

void *malloc(size_t size)
{
    return kmalloc(size);
}
