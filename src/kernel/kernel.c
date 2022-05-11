#include "kernel.h"
#include "../termio/termio.h"
#include "../idt/idt.h"
#include "../memory/kheap.h"
#include "../io/io.h"
#include "../memory/paging.h"
#include "../disk/disk.h"
#include "../disk/disk_stream.h"
#include "../fs/path_parser.h"
#include "../fs/file_table.h"
#include "../string/string.h"
#include "../config.h"
#include "../fs/file.h"
#include "gdt.h"
#include "task/tss.h"
#include "task/task.h"
#include "task/process.h"
#include "panic.h"

extern void *_kernel_end;

void kernel_splash()
{
    print("\n\n\n\n\n");
    terminal_put_str("                  _                                _    ___  __\n", 2);
    terminal_put_str("                 /_\\ __   __ ___    ___  __ _   __| |  /___\\/ _\\\n", 2);
    terminal_put_str("                //_\\\\\\ \\ / // _ \\  / __|/ _` | / _` | //  //\\ \\\n", 2);
    terminal_put_str("               /  _  \\\\ V /| (_) || (__| (_| || (_| |/ \\_// _\\ \\\n", 2);
    terminal_put_str("               \\_/ \\_/ \\_/  \\___/  \\___|\\__,_| \\__,_|\\___/  \\__/\n", 2);
}

struct page_directory_handle kernel_page_directory;

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

void __attribute__((noreturn)) kernel_main()
{
    terminal_init();

    kheap_init();
    paging_init();

    kernel_splash();

    gdt_segments_init_and_load();

    //print("Init kheap\n");

    idt_init();
    //print("Setup interrupts\n");

    tss.esp0 = 0x600000;
    tss.ss0 = GDT_KERNEL_DATA_SEGMENT_SELECTOR;
    tss_load(sizeof(struct gdt_native) * 5);

    file_table_init();
    fs_init();
    disk_init();
    print("\n\nContents of ");
    char path[] = "0:/MOTD.TXT";
    print(path);
    print(":");

    struct FILE *des = fopen(path, "r");
    if (!des) {
        panic("Error opening file\n");
    }

    //char buffer[65536] = { 0 };
    struct stat file_info;
    fstat(des->fileno, &file_info);

    char *buffer = kzalloc(file_info.st_size);

    int read = fread(buffer, 1, file_info.st_size, des);
    if (read < 0) {
        panic("Error reading file\n");
    }

    terminal_init();
    print_char('\n');
    print(buffer);
    enable_interrupts();

    /*
    //kernel_page_directory = paging_init_4gb_directory(PAGING_WRITABLE_PAGE | PAGING_PRESENT | PAGING_ACCESS_FROM_ALL);
    //paging_switch_directory(kernel_page_directory);

    //enable_paging();

    //task_init_initial_task();

    //print_tasks();

    //print("Enabled paging\n");

    //print("Init volume: ");

    file_table_init();
    fs_init();
    disk_init();
    //init_idle_process();
    //enable_interrupts();
    //print("Enabled interrupts\n");
    print("\n\nContents of ");
    char path[] = "0:/MOTD.TXT";
    print(path);
    print(":");

    struct FILE *des = fopen(path, "r");
    if (!des) {
        panic("Error opening file\n");
    }

    char buffer[65536] = { 0 };

    int read = fread(buffer, 1, sizeof(buffer), des);
    struct stat file_info;
    fstat(des->fileno, &file_info);
    if (read < 0) {
        panic("Error reading file\n");
    }

    struct FILE *des2 = fopen(path, "r");
    if (!des) {
        panic("Error opening file\n");
    }

    char buffer2[65536] = { 0 };

    int read2 = fread(buffer2, 1, sizeof(buffer2), des2);
    if (read2 < 0) {
        panic("Error reading file\n");
    }

    int ret = fclose(des);
    if (ret) {
        panic("Error closing file\n");
    }

    print_char('\n');
    print(buffer);
    //size_t in_use = kheap_count_used_blocks();
    //char buffer[10];

    // 1 handler + 1 Table Directory + 1024 Page Tables = 1026
    //itoa(in_use - 1026, buffer);
    //print("\nBlocks in use at exit: ");
    //print(buffer);
    //print_char('\n');
    */

    print("\n\nDone, for now\n");
end:
    goto end;
}
