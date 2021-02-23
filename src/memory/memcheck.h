#ifndef __MEMCHECK_H__
#define __MEMCHECK_H__

#include <stddef.h>
#include <stdint.h>

struct memcheck_entry {
    size_t blocks;
    uintptr_t ptr;
    char filename[50];
    char function[50];
    int line;
};

extern struct memcheck_entry *allocation_table;
extern size_t allocation_table_len;

void memcheck_table_init(int size);
void memcheck_free(void *ptr);
void memcheck_allocate(void *ptr, size_t size, const char * const filename, const char * const function, int line);
void allocation_table_init(int size);
void memcheck_check(char *skip);

#endif
