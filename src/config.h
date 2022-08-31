#ifndef _CONFIG_H
#define _CONFIG_H

#define PAGING_PAGE_SIZE 4096

#define GDT_NULL_SEGMENT 0
#define GDT_KERNEL_CODE_SEGMENT 1
#define GDT_KERNEL_DATA_SEGMENT 2
#define GDT_USER_CODE_SEGMENT 3
#define GDT_USER_DATA_SEGMENT 4
#define GDT_TSS_SEGMENT 5

#define GDT_NATIVE_SIZE 8
#define GDT_NULL_SEGMENT_SELECTOR (0 * GDT_NATIVE_SIZE)
#define GDT_KERNEL_CODE_SEGMENT_SELECTOR (1 * GDT_NATIVE_SIZE)
#define GDT_KERNEL_DATA_SEGMENT_SELECTOR (2 * GDT_NATIVE_SIZE)
#define GDT_USER_CODE_SEGMENT_SELECTOR (3 * GDT_NATIVE_SIZE)
#define GDT_USER_DATA_SEGMENT_SELECTOR (4 * GDT_NATIVE_SIZE)
#define GDT_TSS_SEGMENT_SELECTOR (5 * GDT_NATIVE_SIZE)

#define KERNEL_VIRTUAL_BASE 0xC0000000
//#define KERNEL_HEAP_ADDRESS 0xD0000000
#define KERNEL_MISC_ADDRESS 0xE0000000

#define KERNEL_TOTAL_INTERRUPTS 256

//#define KERNEL_HEAP_SIZE KERNEL_HEAP_ADDRESS - KERNEL_VIRTUAL_BASE
#define KERNEL_HEAP_SIZE 0x1000000 //16M
#define KERNEL_HEAP_BLOCK_SIZE 4096

#define KERNEL_HEAP_TABLE_ADDRESS 0x00007e00
#define PDT_PHYSICAL_ADDR 0xFFFFF000
#define PDT_VIRTUAL_ADDR 0xFFFFF000

#define KERNEL_PAGE_DIRECTORY_VIRTUAL_ADDR 0xC0000000
#define KERNEL_IDT_VIRTUAL_ADDR 0xC0001000
#define KERNEL_GDT_VIRTUAL_ADDR 0xC0002000
#define KERNEL_TSS_VIRTUAL_ADDR 0xC0003000

#define KERNEL_PROCESS_STACK_VIRTUAL_ADDR 0xC000A000

#define MAX_PATH_LEN 250
#define MAX_FILESYSTEMS 5
#define MAX_OPEN_FILES 100

#define GDT_SEGMENTS 6

#define TASK_KERNEL_STACK_SIZE 4096
#define PROGRAM_BASE_VIRT_ADDR 0x400000
#define PROGRAM_STACK_TOP_VIRT_ADDR 0x4000000
#define PROGRAM_STACK_SIZE 4096


#define MAX_PROCESSES 1024

#endif
