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


int strcmp(const char *s1, const char *s2)
{
    while(*s1 && *s2) {
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
    while(*s1 && *s2 && i < n) {
        if (*s1 != *s2) {
            break;
        }
        ++s1;
        ++s2;
        ++i;
    }
    return *s1 - *s2;
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

void itoa(const int c, char *buf)
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

    *(ptr--)  = '\0';

    shifter = is_negative ? -c : c;

    do {
        int digit = shifter % 10;
        *(ptr--) = digit_to_char(digit);
        shifter /= 10;
    } while (shifter);

    if (is_negative){
        *ptr = '-';
    }

}
