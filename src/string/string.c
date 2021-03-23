#include "string.h"
#include "../memory/kheap.h"
#include <stdint.h>

// despite of memset taking an int c, it casts it down to bytes
void *memset(void *s, int c, size_t n)
{
    // Align? Nah
    uint32_t *qword_ptr = (uint32_t *)s;
    size_t dwords = n / sizeof(*qword_ptr);
    size_t remining_bytes = n % sizeof(*qword_ptr);
    char byte = (char)c;
    uint32_t dword_value = byte << 24 | byte << 16 | byte << 8 | byte;

    for (size_t dword = 0; dword < dwords; ++dword) {
        qword_ptr[dword] = dword_value;
    }

    char *byte_ptr = (char *)(s + dwords * 4);
    for (size_t i = 0; i < remining_bytes; ++i) {
        byte_ptr[i] = byte;
    }

    return s;
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
    uint32_t *qword_dest = (uint32_t *)dest;
    uint32_t *qword_source = (uint32_t *)source;

    size_t dwords = n / sizeof(*qword_dest);
    size_t remining_bytes = n % sizeof(*qword_dest);

    for (size_t dword = 0; dword < dwords; ++dword) {
        qword_dest[dword] = qword_source[dword];
    }

    char *dest_reminder = (char *)dest + dwords * sizeof(*qword_dest);
    char *source_reminder = (char *)source + dwords * sizeof(*qword_dest);

    for (size_t i = 0; i < remining_bytes; ++i) {
        dest_reminder[i] = source_reminder[i];
    }

    return dest;
}

void *memmove(void *dest, const void *src, size_t n)
{
    void *buf = kzalloc(n);

    if (!buf) {
        return NULL;
    }

    memcpy(buf, src, n); //NOLINT
    memcpy(dest, buf, n); //NOLINT
    kfree(buf);

    return dest;
}

size_t strlen(const char *const str)
{
    size_t len = 0;
    while (str[len]) {
        ++len;
    }

    return len;
}

size_t strnlen(const char *const str, size_t max_len)
{
    size_t len = 0;
    while (len < max_len && str[len] != '\0') {
        ++len;
    }

    return len;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s2) {
        if (*s1 != *s2) {
            break;
        }
        ++s1;
        ++s2;
    }

    return *s1 - *s2;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    size_t i = 0;
    while (*s1 && *s2) {
        if (*s1 != *s2) {
            break;
        }

        ++i;

        if (i == n) {
            break;
        }

        ++s1;
        ++s2;
    }

    return *s1 - *s2;
}

int strcasecmp(const char *s1, const char *s2)
{
    char char_s1 = 0;
    char char_s2 = 0;

    while (*s1 && *s2) {
        char_s1 = to_upper(*s1);
        char_s2 = to_upper(*s2);

        if (char_s1 != char_s2) {
            break;
        }

        ++s1;
        ++s2;
    }

    return char_s1 - char_s2;
}

int strncasecmp(const char *s1, const char *s2, size_t n)
{
    char char_s1 = 0;
    char char_s2 = 0;
    size_t i = 0;
    while (*s1 && *s2) {
        char_s1 = to_upper(*s1);
        char_s2 = to_upper(*s2);

        if (char_s1 != char_s2) {
            break;
        }

        ++i;

        if (i == n) {
            break;
        }

        ++s1;
        ++s2;
    }

    return *s1 - *s2;
}

char *strcpy(char *dest, const char *src)
{
    char *ptr = dest;

    while (*src) {
        *ptr++ = *src++;
    }

    *ptr = '\0';
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n)
{
    size_t i;

    for (i = 0; i < n && src[i]; ++i) {
        dest[i] = src[i];
    }

    for (; i < n; ++i) {
        dest[i] = '\0';
    }

    return dest;
}

bool is_digit(const char c)
{
    return c >= '0' && c <= '9';
}

int numeric_char_to_digit(const char c)
{
    return c - '0';
}

char digit_to_char(const int c)
{
    if (c < 0 || c > 9) {
        return 'E';
    }

    return '0' + c;
}

void itoa(const int64_t c, char *buf)
{
    int shifter = c;
    char *ptr = buf;
    if (shifter == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    int is_negative = 0;

    if (shifter < 0) {
        ++ptr;
        is_negative = 1;
        shifter *= -1;
    }

    //no reverse string for now
    do {
        shifter /= 10;
        ++ptr;
    } while (shifter);

    *(ptr--) = '\0';

    shifter = is_negative ? -c : c;

    do {
        int digit = shifter % 10;
        *(ptr--) = digit_to_char(digit);
        shifter /= 10;
    } while (shifter);

    if (is_negative) {
        *ptr = '-';
    }
}

char *strchr(const char *s, int c)
{
    while (*s) {
        if (c == *s) {
            return (char *)s;
        }
        ++s;
    }

    return NULL;
}

int to_upper(const char c)
{
    return (c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c;
}

int to_lower(const char c)
{
    return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}
