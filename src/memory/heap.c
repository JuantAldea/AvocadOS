#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "heap.h"
#include "../config.h"
#include "../status.h"
#include "../termio/termio.h"
#include "../string/string.h"

//#define COUNT_ALLOCATIONS
//#define COUNT_FREES

static int heap_validate_table(void *ptr, void *end, struct heap_table *table)
{
    const size_t table_size = (size_t)(end - ptr);
    const size_t total_blocks = table_size / KERNEL_HEAP_BLOCK_SIZE;
    if (table->len != total_blocks) {
        return -EINVAL;
    }

    return KERNEL_OK;
}

static bool heap_validate_alignment(void *ptr)
{
    return ((uintptr_t)ptr % KERNEL_HEAP_BLOCK_SIZE) == 0;
}

int heap_create(struct heap *heap, void *ptr, void *end_addr, struct heap_table *heap_table)
{
    if (!heap_validate_alignment(ptr) || !heap_validate_alignment(end_addr)) {
        return -EINVAL;
    }

    memset(heap, 0, sizeof(struct heap)); // NOLINT
    heap->base_addr = ptr;
    heap->table = heap_table;

    if (heap_validate_table(ptr, end_addr, heap_table)) {
        return -EINVAL;
    }

    const size_t table_size = sizeof(HEAP_TABLE_ENTRY) * heap_table->len;
    memset(heap_table->entries, HEAP_MEMORY_BLOCK_FREE, table_size); // NOLINT

    return KERNEL_OK;
}

size_t size_to_nblocks(size_t size)
{
    return size / KERNEL_HEAP_BLOCK_SIZE + ((size % KERNEL_HEAP_BLOCK_SIZE) ? 1 : 0);
}

int find_first_block(struct heap *heap, size_t n_blocks)
{
    struct heap_table *heap_table = heap->table;
    HEAP_TABLE_ENTRY *heap_entries = heap_table->entries;

    int remaining_blocks = n_blocks;
    int found_block = -1;

    for (size_t i = 0; (i < heap_table->len) && remaining_blocks; ++i) {
        if (BLOCK_TEST_TAKEN(heap_entries[i])) {
            found_block = -1;
            remaining_blocks = n_blocks;
            continue;
        }

        if (found_block == -1) {
            found_block = i;
        }

        --remaining_blocks;
    }

    if (found_block == -1) {
        return -ENOMEM;
    }

    return found_block;
}

void heap_reserve_blocks(struct heap *heap, size_t first_block, size_t n_blocks)
{
    HEAP_TABLE_ENTRY *heap_entries = heap->table->entries;
    const size_t end_block = first_block + n_blocks;

    for (size_t j = first_block; j < end_block; ++j) {
        BLOCK_SET_TAKEN(heap_entries[j]);
        BLOCK_SET_NOT_FIRST(heap_entries[j]);
        BLOCK_SET_NEXT(heap_entries[j]);
    }

    //set the first block to first
    BLOCK_SET_FIRST(heap_entries[first_block]);
    //set the last has NO-NEXT
    BLOCK_SET_NO_NEXT(heap_entries[end_block - 1]);
}

void *heap_block_to_addr(const struct heap *const heap, size_t block_index)
{
    return heap->base_addr + block_index * KERNEL_HEAP_BLOCK_SIZE;
}

size_t heap_addr_to_block(const struct heap *const heap, void *ptr)
{
    return (size_t)(ptr - heap->base_addr) / KERNEL_HEAP_BLOCK_SIZE;
}

void *heap_malloc_blocks(struct heap *heap, size_t n_blocks)
{
    int first_block = find_first_block(heap, n_blocks);

    if (first_block < 0) {
        return NULL;
    }

    heap_reserve_blocks(heap, first_block, n_blocks);
#ifdef COUNT_ALLOCATIONS
    print("[HEAP] Reserved: ");
    char buff[10];
    itoa(n_blocks, buff);
    print(buff);
    print_char('\n');
#endif

    return heap_block_to_addr(heap, first_block);
}

void *heap_malloc(struct heap *heap, size_t size)
{
    size_t n_blocks = size_to_nblocks(size);

    if (!n_blocks) {
        // no 0-long regions
        return NULL;
    }

    return heap_malloc_blocks(heap, n_blocks);
}

void heap_free(struct heap *heap, void *ptr)
{
    struct heap_table *heap_table = heap->table;

    HEAP_TABLE_ENTRY *heap_entries = heap_table->entries;

    size_t block = heap_addr_to_block(heap, ptr);

    if (!BLOCK_TEST_FIRST(heap_entries[block])) {
        //int?
        return;
    }
#ifdef COUNT_FREES
    size_t released_blocks = 1;
#endif
    BLOCK_SET_FREE(heap_entries[block++]);
    while (!BLOCK_TEST_FREE(heap_entries[block]) && !BLOCK_TEST_FIRST(heap_entries[block])) {
        BLOCK_SET_FREE(heap_entries[block]);
        ++block;
#ifdef COUNT_FREES
        ++released_blocks;
#endif
    }
#ifdef COUNT_FREES
    print("[HEAP] Released: ");
    char buff[10];
    itoa(released_blocks, buff);
    print(buff);
    print_char('\n');
#endif
}

size_t count_used_blocks(struct heap *heap)
{
    struct heap_table *heap_table = heap->table;
    size_t count = 0;
    for (size_t i = 0; i < heap_table->len; ++i) {
        if (BLOCK_TEST_TAKEN(heap_table->entries[i])) {
            ++count;
        }
    }
    return count;
}
