#ifndef __STRING_H
#define __STRING_H

#include <stddef.h>
#include <stdbool.h>

size_t strlen(const char * const str);
size_t strnlen(const char * const str, size_t max_len);
bool is_digit(const char c);
int numeric_char_to_digit(const char c);
char digit_to_char(const int c);
#endif