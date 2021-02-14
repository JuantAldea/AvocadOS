#ifndef __HEAP_H
#define __HEAP_H
#include <stddef.h>
#include <stdint.h>
#define HEAP_MEMORY_BLOCK_FREE 0x0
#define HEAP_MEMORY_BLOCK_TAKEN 0x1

#define HEAP_MEMORY_BLOCK_HAS_NEXT 0b10000000
#define HEAP_MEMORY_BLOCK_IS_FIRST 0b01000000

#define BLOCK_TEST_FREE(b)  (b == HEAP_MEMORY_BLOCK_FREE)
#define BLOCK_TEST_TAKEN(b)  (b & HEAP_MEMORY_BLOCK_TAKEN)
#define BLOCK_TEST_HAS_NEXT(b) (b & HEAP_MEMORY_BLOCK_HAS_NEXT)

#define BLOCK_TEST_FIRST(b) (b & HEAP_MEMORY_BLOCK_IS_FIRST)
#define BLOCK_TEST_NOT_FIRST(b) (b & ~HEAP_MEMORY_BLOCK_IS_FIRST)

#define BLOCK_SET_TAKEN(b) (b = b | HEAP_MEMORY_BLOCK_TAKEN)
#define BLOCK_SET_FREE(b) (b = HEAP_MEMORY_BLOCK_FREE)
#define BLOCK_SET_NEXT(b)  (b = b | HEAP_MEMORY_BLOCK_HAS_NEXT)
#define BLOCK_SET_NO_NEXT(b)  (b = b & ~HEAP_MEMORY_BLOCK_HAS_NEXT)
#define BLOCK_SET_FIRST(b) (b = b | HEAP_MEMORY_BLOCK_IS_FIRST)
#define BLOCK_SET_NOT_FIRST(b) (b = b & ~HEAP_MEMORY_BLOCK_IS_FIRST)

typedef uint8_t HEAP_TABLE_ENTRY;

struct heap_table {
    HEAP_TABLE_ENTRY *entries;
    size_t len;
};

struct heap {
    struct heap_table *table;
    void *base_addr;
};

int heap_create(struct heap *heap, void *addr, void *end_addr, struct heap_table *heap_table);

void *heap_malloc(struct heap *heap, size_t size);
void heap_free(struct heap *heap, void *ptr);
size_t count_used_blocks(struct heap *heap);

#endif
