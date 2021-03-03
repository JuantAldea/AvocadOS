#ifndef _PAGING_H
#define _PAGING_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// clang-format off

#define PAGING_CACHE_DISABLE     0b00010000
#define PAGING_WRITE_THROUGH     0b00001000
#define PAGING_ACCESS_FROM_ALL   0b00000100
#define PAGING_WRITABLE_PAGE     0b00000010
#define PAGING_PRESENT           0b00000001

#define PAGING_ENTRIES_PER_TABLE 1024
#define PAGING_PAGE_SIZE 4096
#define ADDR_MASK 0xfffff000

// clang-format on

struct page_directory_handle {
    uintptr_t *directory;
};

struct page_directory_handle *paging_init_4gb_directory(const uint8_t flags);
void paging_free_4gb_directory(struct page_directory_handle *handle);
void paging_switch_directory(const struct page_directory_handle *const chunk);
extern void enable_paging();

bool paging_addr_is_page_aligned(const void *const addr);
void *paging_align_address(const void *const addr);

int paging_map_page(struct page_directory_handle *chunk, const void *const virt, const void *const phys, const uint8_t flags);
int paging_map_from_to(struct page_directory_handle *const directory, void *virt, void *phys_begin, void *phys_end, int flags);
int paging_map_range(struct page_directory_handle *const directory, void *virt, void *phys, int count, int flags);
//int paging_map(struct page_directory_handle *const directory, void *virt, void *phys, int flags);

#endif
