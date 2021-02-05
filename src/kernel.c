#include "kernel.h"
#include "termio/termio.h"
#include "idt/idt.h"
#include "memory/memory.h"
#include "io/io.h"

void kernel_splash()
{
    print("\n\n\n\n\n");
    terminal_put_str("                  _                                _    ___  __\n", 2);
    terminal_put_str("                 /_\\ __   __ ___    ___  __ _   __| |  /___\\/ _\\\n", 2);
    terminal_put_str("                //_\\\\\\ \\ / // _ \\  / __|/ _` | / _` | //  //\\ \\\n", 2);
    terminal_put_str("               /  _  \\\\ V /| (_) || (__| (_| || (_| |/ \\_// _\\ \\\n", 2);
    terminal_put_str("               \\_/ \\_/ \\_/  \\___/  \\___|\\__,_| \\__,_|\\___/  \\__/\n", 2);
}


void kernel_main(void){
    terminal_init();
    kernel_splash();
    print("Starting...\n");

    idt_init();
    enable_interrupts();

trap:
    goto trap;
}
