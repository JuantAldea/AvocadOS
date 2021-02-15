#ifndef __TERMIO_H
#define __TERMIO_H

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#include <stdint.h>
#include <stddef.h>

enum VGA_COLOUR
{
    VGA_COLOUR_BLACK = 0,
    VGA_COLOUR_BLUE,
    VGA_COLOUR_GREEN,
    VGA_COLOUR_CYAN,
    VGA_COLOUR_RED,
    VGA_COLOUR_MAGENTA,
    VGA_COLOUR_BROWN,
    VGA_COLOUR_LIGHT_GRAY,
    VGA_COLOUR_DARK_GRAY,
    VGA_COLOUR_LIGHT_BLUE,
    VGA_COLOUR_LIGHT_GREEN,
    VGA_COLOUR_LIGHT_CYAN,
    VGA_COLOUR_LIGHT_RED,
    VGA_COLOUR_LIGHT_MAGENTA,
    VGA_COLOUR_YELLOW,
    VGA_COLOUR_WHITE,
};

void terminal_put_char(const uint8_t character, const enum VGA_COLOUR colour);
uint16_t terminal_make_char(const char c,  const enum VGA_COLOUR);

void terminal_print_char_x_y(const uint16_t character, const uint16_t x, const uint16_t y);
void terminal_print_string_x_y(const char *str,  const enum VGA_COLOUR, const uint8_t x, const uint8_t y);
void terminal_put_str(const char *str,  const enum VGA_COLOUR);
void terminal_init();
void print(const char *const str);
void print_char(const char chr);

#endif /* __TERMIO_H*/
