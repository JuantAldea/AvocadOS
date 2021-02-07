#include "kernel.h"
#include "termio/termio.h"
#include "idt/idt.h"
#include "memory/memory.h"
#include "memory/kheap.h"
#include "io/io.h"
#include "memory/paging.h"

void kernel_splash()
{
    print("\n\n\n\n\n");
    terminal_put_str("                  _                                _    ___  __\n", 2);
    terminal_put_str("                 /_\\ __   __ ___    ___  __ _   __| |  /___\\/ _\\\n", 2);
    terminal_put_str("                //_\\\\\\ \\ / // _ \\  / __|/ _` | / _` | //  //\\ \\\n", 2);
    terminal_put_str("               /  _  \\\\ V /| (_) || (__| (_| || (_| |/ \\_// _\\ \\\n", 2);
    terminal_put_str("               \\_/ \\_/ \\_/  \\___/  \\___|\\__,_| \\__,_|\\___/  \\__/\n", 2);
}

static struct page_directory_handle *kernel_chunk = NULL;

void kernel_main(void)
{
    terminal_init();
    kernel_splash();
    print("Starting...\n");

    kheap_init();

    idt_init();

    kernel_chunk = page_directory_init_4gb(PAGING_WRITABLE_PAGE
                    | PAGING_PRESENT
                    | PAGING_ACCESS_FROM_ALL);
    paging_switch_directory(kernel_chunk);

    char *page_ptr = kzalloc(4096);

    // map the region pointed by page_ptr to virtual addr 0x1000

    paging_map_v_addr(kernel_chunk, (void*) 0x1000, page_ptr,
                            PAGING_ACCESS_FROM_ALL
                            | PAGING_PRESENT
                            | PAGING_WRITABLE_PAGE);

    // also to addr 0x2000
    paging_map_v_addr(kernel_chunk, (void*) 0x2000, page_ptr,
                            PAGING_ACCESS_FROM_ALL
                            | PAGING_PRESENT
                            | PAGING_WRITABLE_PAGE);

    char *ptr = (void*)0x1000;
    ptr[0] = '1';
    ptr[1] = '2';

    // before mapping
    print("Paging test\n");
    print("0x1000 and 0x2000 virtual addresses are both mapped\nto the same physical page \"page_ptr\".\n");
    print("####################################################\n");

    print("[0x1000] before mapping: ");
    print(ptr);
    print("\n");

    enable_paging();

    // write to virtual address 0x1000
    print("[0x1000] after mapping: ");
    print(ptr);
    print("\n");

    ptr[0] = 'A';
    ptr[1] = 'B';
    print("[0x1000] after writing \"AB\" into it: ");
    print(ptr);
    print("\n");

    /* read it from 0x2000 as well */
    char *ptr2 = (void*)0x2000;
    print("[0x2000]: ");
    print(ptr2);
    print("\n");
    // page_ptr is still mapped linearly
    print("[page_ptr]: ");
    print(page_ptr);
    print("\n");
    print("####################################################\n");
    enable_interrupts();

trap:
    goto trap;
}
