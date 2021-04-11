#ifndef _KHEAP_H
#define _KHEAP_H

#include <stddef.h>
#include "heap.h"

//#define MEMCHECK

#ifdef MEMCHECK
#define kzalloc(X) __memcheck_kzalloc(X, __FILE__, __FUNCTION__, __LINE__)
#define kfree(X) __memcheck_kfree(X)
#else
#define kzalloc(X) __kzalloc(X)
#define kfree(X) __kfree(X)
#endif

extern struct heap kernel_heap;
extern struct heap_table kheap_table;

void kheap_init();
void *kmalloc(size_t size);
void *__kzalloc(size_t size);
void __kfree(void *ptr);
size_t kheap_count_used_blocks();
void kheap_stats();

size_t kheap_addr_to_block(void *ptr);
void *__memcheck_kzalloc(size_t size, const char *filename, const char *function, int line);
void __memcheck_kfree(void *ptr);
size_t kheap_used();
size_t kheap_free();
// for GDB
void *malloc(size_t size);

#endif
