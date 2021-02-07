#ifndef _PAGING_H
#define _PAGING_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PAGING_CACHE_DISABLE     0b00010000
#define PAGING_WRITE_THROUGH     0b00001000
#define PAGING_ACCESS_FROM_ALL   0b00000100
#define PAGING_WRITABLE_PAGE     0b00000010
#define PAGING_PRESENT           0b00000001

#define PAGING_ENTRIES_PER_TABLE 1024
#define PAGING_PAGE_SIZE 4096
#define ADDR_MASK 0xfffff000

struct page_directory_handle
{
    uint32_t *directory;
};

struct page_directory_handle *page_directory_init_4gb(const uint8_t flags);
void paging_switch_directory (const struct page_directory_handle * const chunk);
extern void enable_paging();

bool paging_addr_is_page_aligned (const void * const addr);
int paging_map_v_addr(struct page_directory_handle *chunk, const void * const virtual_addr, const void *const page_ptr, const uint32_t flags);

#endif
