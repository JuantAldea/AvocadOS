#ifndef _PAGING_H
#define _PAGING_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// clang-format off

#define PAGING_CACHE_DISABLE     16 //0b00010000
#define PAGING_WRITE_THROUGH     8 //0b00001000
#define PAGING_ACCESS_FROM_ALL   4 //0b00000100
#define PAGING_WRITABLE_PAGE     2 //0b00000010
#define PAGING_PRESENT           1 //0b00000001

#define PSE_BIT 0x00000010

#define PAGING_ENTRIES_PER_TABLE 1024
#define PAGING_PAGE_SIZE 4096
#define ADDR_MASK 0xfffff000

#define V_ADDR_PAGEDIR_INDEX(vaddr) (((uintptr_t)vaddr) >> 22)
#define V_ADDR_PAGETBL_INDEX(vaddr) ((((uintptr_t)vaddr) >> 12) & 0x3ff)
#define V_ADDR_PAGEFRAME_INDEX(vaddr) (((uintptr_t)vaddr) & 0xfff)

#define V_ADDR_PAGEFRAME_INDEX_PSE(vaddr) (((uintptr_t)vaddr) & 0x3fffff)
#define PAGE_ENTRY_PSE_BIT(page_directory_entry) (((uintptr_t)page_directory_entry) & 0x80)
#define PAGE_ENTRY_ADDR(page_entry) ((uintptr_t)(((uintptr_t)page_entry) & ADDR_MASK))
#define PAGE_ENTRY_FLAGS(page_entry) ((uintptr_t)(((uintptr_t)page_entry) & ~ADDR_MASK))
#define PAGE_BITMAP_INDEX(page_addr) (((uintptr_t)page_addr - (uintptr_t)bitmap_begin) >> 12)

extern struct page_directory_handle *current_directory;

struct page_dir_entry {
    /*
    unsigned int present    : 1;
    unsigned int rw         : 1;
    unsigned int user       : 1;
    unsigned int w_through  : 1;
    unsigned int cache      : 1;
    unsigned int access     : 1;
    unsigned int reserved   : 1;
    unsigned int page_size  : 1;
    unsigned int global     : 1;
    unsigned int available  : 3;
    */
    uintptr_t flags     : 12;
    uintptr_t frame      : 20;
} __attribute__((packed));

struct page_table_entry {
    /*
    unsigned int present    : 1;
    unsigned int rw         : 1;
    unsigned int user       : 1;
    unsigned int reserved   : 2;
    unsigned int accessed   : 1;
    unsigned int dirty      : 1;
    unsigned int reserved  : 2;
    unsigned int available  : 3;
    */
    uintptr_t flags     : 12;
    uintptr_t frame      : 20;
} __attribute__((packed));

struct page_table
{
    uintptr_t *pages;
};

/*
typedef struct {
    uintptr_t physical_addr;
} physical_addr_t;
*/

struct page_directory_handle {
    uintptr_t *directory;
    uintptr_t *directory_virtual[1024];
} __attribute__((packed));

struct page_directory_handle *paging_init_4gb_directory(const uint16_t flags);
//void paging_free_4gb_directory(struct page_directory_handle *handle);
void paging_switch_directory(struct page_directory_handle *const chunk);
//extern void enable_paging();
void paging_init();
bool paging_addr_is_page_aligned(const void *const addr);
uintptr_t paging_align_address(uintptr_t addr);

int paging_map_page(struct page_directory_handle *handle, uintptr_t virt_addr, uintptr_t page_ptr, const uint16_t flags);
int paging_map_from_to(struct page_directory_handle *const handle, uintptr_t virt_addr, uintptr_t phys_begin, uintptr_t phys_end, int flags);
int paging_map_range(struct page_directory_handle *const handle, uintptr_t virt_addr, uintptr_t phys_addr, int count, int flags);
struct page_directory_handle *paging_clone_directory(struct page_directory_handle *source);
uintptr_t virtual_to_physical_addr(struct page_directory_handle *handle, void *virtual_addr);
#endif
