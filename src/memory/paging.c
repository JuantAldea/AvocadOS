#include "paging.h"
#include "kheap.h"
#include "../status.h"

static uint32_t *current_directory = NULL;

extern void paging_load_directory(const uint32_t * const directory);

struct page_directory_handle *page_directory_init_4gb(const uint8_t flags)
{
    struct page_directory_handle* chunk = kzalloc(sizeof(struct page_directory_handle));

    chunk->directory = kzalloc(sizeof(uint32_t) * PAGING_ENTRIES_PER_TABLE);
    for (int i = 0; i < PAGING_ENTRIES_PER_TABLE; ++i) {
        uint32_t * const page_table_entry = kzalloc(sizeof(uint32_t) * PAGING_ENTRIES_PER_TABLE);
        chunk->directory[i] = (uint32_t) page_table_entry | flags | PAGING_WRITABLE_PAGE;
        const int directory_offset = i * PAGING_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE;
        for (int j = 0; j < PAGING_ENTRIES_PER_TABLE; ++j) {
            // linear mapping
            const uint32_t page_addr = (directory_offset + (j * PAGING_PAGE_SIZE ));
            page_table_entry[j] = page_addr | flags;
        }

    }
    return chunk;
}

bool paging_addr_is_page_aligned (const void * const addr)
{
    return (uint32_t)addr % PAGING_PAGE_SIZE == 0;
}

void paging_switch_directory (const struct page_directory_handle * const chunk)
{
    paging_load_directory(chunk->directory);
    current_directory = chunk->directory;
}

int paging_v_addr_to_paging_indexes (const void * const virtual_addr, uint32_t * const directory_index_out, uint32_t * const table_index_out)
{
    if (!paging_addr_is_page_aligned(virtual_addr)) {
        return -EINVAL;
    }

    *directory_index_out = (uint32_t) virtual_addr / (PAGING_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);
    const uint32_t offset = (uint32_t) virtual_addr % (PAGING_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);
    *table_index_out = offset / PAGING_PAGE_SIZE;

    return 0;
}

int paging_map_v_addr(struct page_directory_handle *const chunk, const void * const virtual_addr, const void *const page_ptr, const uint32_t flags)
{
    if (!paging_addr_is_page_aligned(virtual_addr) || !paging_addr_is_page_aligned(page_ptr)) {
        return -EINVAL;
    }

    uint32_t directory_index;
    uint32_t table_index;
    int ret = 0;
    if((ret = paging_v_addr_to_paging_indexes(virtual_addr, &directory_index, &table_index)) < 0) {
        return ret;
    }

    const uint32_t page_directory_entry = chunk->directory[directory_index];
    // extract Page Table addr from page_directory_entry
    uint32_t * const page_table = (uint32_t*) (page_directory_entry & ADDR_MASK);
    // set the Page table entry with the target addr its flags and all that
    page_table[table_index] = (uint32_t)page_ptr | flags;
    return 0;
}
