
#include "../termio/termio.h"

void panic(const char *const msg)
{
    terminal_put_str("[PANIC] ", VGA_COLOUR_LIGHT_RED);
    terminal_put_str(msg, VGA_COLOUR_WHITE);
trap:
    goto trap;
}