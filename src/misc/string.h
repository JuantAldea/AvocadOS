#ifndef __STRING_H
#define __STRING_H

#include <stddef.h>

size_t strlen(const char * const str);
size_t strnlen(const char * const str, size_t max_len);

#endif