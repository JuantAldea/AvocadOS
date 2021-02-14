#include "kernel.h"
#include "../termio/termio.h"
#include "../idt/idt.h"
#include "../memory/memory.h"
#include "../memory/kheap.h"
#include "../io/io.h"
#include "../memory/paging.h"
#include "../disk/disk.h"
#include "../disk/disk_stream.h"
#include "../fs/path_parser.h"
#include "../fs/vfs.h"
#include "../string/string.h"

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

void print_path(struct path_root *path)
{
    struct path_part *part = path->first;
    print("Drive: ");
    char buffer[10];
    itoa(path->drive_number, buffer);
    print(buffer);
    print_char('\n');
    while (part) {
        print(part->part);
        print("\n");
        part = part->next;
    }
}

void kernel_main(void)
{
    terminal_init();
    kernel_splash();

    kheap_init();

    print("Init kheap\n");

    idt_init();
    print("Setup interrupts\n");

    kernel_chunk = page_directory_init_4gb(PAGING_WRITABLE_PAGE | PAGING_PRESENT | PAGING_ACCESS_FROM_ALL);
    paging_switch_directory(kernel_chunk);
    enable_paging();

    print("Enabled paging\n");

    print("Init volume: ");
    vfs_init();
    disk_init();

    enable_interrupts();
    print("Enabled interrupts\n");

    size_t in_use = kheap_count_used_blocks();
    char buffer[10];

    // 1 handler + 1 Table Directory + 1024 Page Tables = 1026
    itoa(in_use - 1026, buffer);
    print("\nBlocks in use at exit: ");
    print(buffer);
    print_char('\n');

    print("\n\nDone, for now\n");
trap:
    goto trap;
}
