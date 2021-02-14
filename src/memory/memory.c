#include "memory.h"

// despite of memset taking an int c, it casts it down to bytes
void *memset(void *s, int c, size_t n)
{
    char *ptr = (char *)s;

    for (size_t i = 0; i < n; ++i) {
        ptr[i] = (char)c;
    }

    return ptr;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        if (((char *)s1)[i] != ((char *)s2)[i]) {
            return ((char *)s1)[i] != ((char *)s2)[i];
        }
    }
    return 0;
}

void *memchr(const void *s, int c, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        if (c == ((char *)s)[i]) {
            return (void *)(s + i);
        }
    }
    return NULL;
}

void *memcpy(void *dest, const void *source, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        ((char *)dest)[i] = ((char *)source)[i];
    }

    return dest;
}
