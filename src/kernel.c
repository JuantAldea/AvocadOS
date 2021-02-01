#include "kernel.h"
#include "termio.h"

void kernel_main(void)
{
    terminal_initialize();

    print("Would you fancy an avocado?");

trap:
    goto trap;
}
