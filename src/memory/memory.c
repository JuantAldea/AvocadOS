#include "memory.h"

// despite of memset taking an int c, it casts it down to bytes
void* memset(void *s, int c, size_t n)
{
    char *ptr = (char*) s;

    for (size_t i = 0; i < n; ++i) {
        ptr[i] = (char) c;
    }

    return ptr;
}
