#ifndef __MEMORY_H
#define __MEMORY_H

#include <stddef.h>

void* memset(void *ptr, int c, size_t size);
int memcmp(const void *s1, const void *s2, size_t n);
void *memchr(const void *s, int c, size_t n);
void *memcpy(void *dest, const void *source, size_t n);
#endif /*__MEMORY_H*/
