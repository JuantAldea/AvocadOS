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
#include "task/task.h"
#include "task/process.h"
#include "panic.h"

//extern void *_kernel_start;
void remap_master_pic_C(void)
{
    // https://pdos.csail.mit.edu/6.828/2018/readings/hardware/8259A.pdf
    // https://wiki.osdev.org/PIC#Protected_Mode
    //unsigned char a1 = insb(PIC1_DATA_PORT);                        // save masks
    //unsigned char a2 = insb(PIC2_DATA_PORT);
    //ICW1
    outb(PIC1_CMD_PORT, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    io_delay();
    outb(PIC2_CMD_PORT, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    io_delay();

    //ICW2
    /*
        A common choice is to move them to the beginning of the available range
        (IRQs 0..0xF -> INT 0x20..0x2F).
        For that, we need to set the master PIC's
        base for PIC1 ->0x20
        base for PIC2 ->0x28
    */
    outb(PIC1_DATA_PORT, 0x20);
    io_delay();
    outb(PIC2_DATA_PORT, 0x28);
    io_delay();

    //ICW3
    outb(PIC1_DATA_PORT, PIC_ICW3_SLAVE_AT_IRQ2);
    io_delay();
    outb(PIC2_DATA_PORT, PIC_ICW3_SLAVE_ID);
    io_delay();

    //ICW4
    outb(PIC1_DATA_PORT, PIC_ICW4_8086_MODE);
    io_delay();
    outb(PIC2_DATA_PORT, PIC_ICW4_8086_MODE);
    io_delay();
    outb(PIC1_DATA_PORT, 0x0);
    outb(PIC2_DATA_PORT, 0x0);

    //outb(PIC1_DATA_PORT, a1);   // restore saved masks.
    //outb(PIC2_DATA_PORT, a2);

    return;
}

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

void system_function()
{
    asm volatile("nop");
}

void __attribute__((noreturn)) kernel_main()
{
    remap_master_pic_C();
    gdt_init();
    idt_init();
    terminal_init();
    kheap_init();

    paging_init();

    terminal_init();

    kernel_splash();

    file_table_init();
    fs_init();
    disk_init();
    tasking_init();

    /*
    print("\n\nContents of ");
    char path[] = "0:/MOTD.TXT";
    print(path);
    print(":");

    struct FILE *des = fopen(path, "r");
    if (!des) {
        panic("Error opening file\n");
    }

    //char buffer[65536] = { 0 };
    char *buffer = kzalloc(file_info.st_size);

    int read = fread(buffer, 1, file_info.st_size, des);
    if (read < 0) {
        panic("Error reading file\n");
    }

    terminal_init();
    print_char('\n');
    print(buffer);
    //char buffer2[65536] = { 0 };
    //(void) buffer2;

    */
    /*
    //print_tasks();

    //print("Enabled paging\n");

    //print("Init volume: ");
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

    int read2 = fread(buffer2, 1, sizeof(buffer2), des2)
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
    /*
    print_char('\n');
    for (int i = 0; i < 5; i++) {
        itoa(i, buffer);
        print("System - ");
        print(buffer);
        print("                         \n");
    }
    */
    while (1) {
        system_function();
    }
    asm volatile("hlt");

    goto end;
}
