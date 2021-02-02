#include "kernel.h"
#include "termio/termio.h"
#include "idt/idt.h"

void kernel_splash()
{
    print("\n\n\n\n\n");
    terminal_put_str("                  _                                _    ___  __\n", 2);
    terminal_put_str("                 /_\\ __   __ ___    ___  __ _   __| |  /___\\/ _\\\n", 2);
    terminal_put_str("                //_\\\\\\ \\ / // _ \\  / __|/ _` | / _` | //  //\\ \\\n", 2);
    terminal_put_str("               /  _  \\\\ V /| (_) || (__| (_| || (_| |/ \\_// _\\ \\\n", 2);
    terminal_put_str("               \\_/ \\_/ \\_/  \\___/  \\___|\\__,_| \\__,_|\\___/  \\__/\n", 2);
}

void test_div0()
{
    volatile int a = 0;
    volatile int b = a / a;
    (void)b;
}

void kernel_main(void){
    terminal_initialize();
    //print("Starting...");

    idt_init();

    // done
    kernel_splash();
    //test_div0();


trap:
    goto trap;
}
