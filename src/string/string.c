#include "string.h"

size_t strlen(const char  * const str)
{
    size_t len = 0;
    while (str[len]) {
        ++len;
    }

    return len;
}

size_t strnlen(const char  * const str, size_t max_len)
{
    size_t len = 0;
    while (len < max_len && str[len++] != '\0') { ; }
    return len;
}

bool is_digit(const char c)
{
    return c > '0' && c < '9';
}

int numeric_char_to_digit(const char c)
{
    return c - '0';
}
