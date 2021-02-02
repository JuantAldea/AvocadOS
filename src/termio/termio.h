#ifndef __TERMIO_H
#define __TERMIO_H

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#include <stdint.h>
#include <stddef.h>


void terminal_put_char(const uint8_t character, uint8_t colour);
uint16_t terminal_make_char(const char c, const char colour);

size_t strlen(const char  * str);
void terminal_print_char_x_y(const uint16_t character, const uint16_t x, const uint16_t y);
void terminal_print_string_x_y(const char *str, const char colour, const uint8_t x, const uint8_t y);
void terminal_put_str(const char *str, const char colour);
void terminal_initialize ();
void print(const char *str);

#endif /* __TERMIO_H*/
