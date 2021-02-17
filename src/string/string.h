#ifndef __STRING_H
#define __STRING_H

#include <stddef.h>
#include <stdbool.h>

void* memset(void *ptr, int c, size_t size);
int memcmp(const void *s1, const void *s2, size_t n);
void *memchr(const void *s, int c, size_t n);
void *memcpy(void *dest, const void *source, size_t n);
void *memmove(void *dest, const void *src, size_t n);

size_t strlen(const char *const str);
size_t strnlen(const char *const str, size_t max_len);

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

char *strncpy(char *dest, const char *src, size_t n);
char *strcpy(char *dest, const char *src);

char *strchr(const char *s, int c);

bool is_digit(const char c);
int numeric_char_to_digit(const char c);
char digit_to_char(const int c);
void itoa(const int c, char *buf);

#endif
