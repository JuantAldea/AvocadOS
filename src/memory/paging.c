#include "paging.h"
#include "kheap.h"
#include "../status.h"

static uintptr_t *current_directory = NULL;

extern void paging_load_directory(const uintptr_t *const directory);

struct page_directory_handle *paging_init_4gb_directory(const uint8_t flags)
{
    struct page_directory_handle *handle = kzalloc(sizeof(struct page_directory_handle));

    if (!handle) {
        return NULL;
    }

    handle->directory = kzalloc(sizeof(uintptr_t) * PAGING_ENTRIES_PER_TABLE);
    for (int i = 0; i < PAGING_ENTRIES_PER_TABLE; ++i) {
        uintptr_t *const page_table_entry = kzalloc(sizeof(uintptr_t) * PAGING_ENTRIES_PER_TABLE);
        handle->directory[i] = (uintptr_t)page_table_entry | flags | PAGING_WRITABLE_PAGE;
        const int directory_offset = i * PAGING_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE;
        for (int j = 0; j < PAGING_ENTRIES_PER_TABLE; ++j) {
            // linear mapping
            const uintptr_t page_addr = (directory_offset + (j * PAGING_PAGE_SIZE));
            page_table_entry[j] = page_addr | flags;
        }
    }

    return handle;
}

void paging_free_4gb_directory(struct page_directory_handle *handle)
{
    for (int i = 0; i < PAGING_ENTRIES_PER_TABLE; ++i) {
        uintptr_t entry_addr = handle->directory[i] & 0xfffff000;
        kfree((void *)entry_addr);
    }

    kfree(handle->directory);
    kfree(handle);
}

bool paging_addr_is_page_aligned(const void *const addr)
{
    return (uintptr_t)addr % PAGING_PAGE_SIZE == 0;
}

void *paging_align_address(const void *const addr)
{
    return (void *)(PAGING_PAGE_SIZE * (((uintptr_t)addr + (PAGING_PAGE_SIZE - 1)) / PAGING_PAGE_SIZE));
}

void paging_switch_directory(const struct page_directory_handle *const handle)
{
    paging_load_directory(handle->directory);
    current_directory = handle->directory;
}

int paging_v_addr_to_paging_indexes(const void *const virt_addr, uintptr_t *const directory_index_out, uintptr_t *const table_index_out)
{
    if (!paging_addr_is_page_aligned(virt_addr)) {
        return -EINVAL;
    }

    *directory_index_out = (uintptr_t)virt_addr / (PAGING_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);
    const uintptr_t offset = (uintptr_t)virt_addr % (PAGING_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);
    *table_index_out = offset / PAGING_PAGE_SIZE;

    return 0;
}

int paging_map_page(struct page_directory_handle *const handle, const void *const virt_addr, const void *const page_ptr,
                    const uint8_t flags)
{
    if (!paging_addr_is_page_aligned(virt_addr) || !paging_addr_is_page_aligned(page_ptr)) {
        return -EINVAL;
    }

    uintptr_t directory_index;
    uintptr_t table_index;

    int res = 0;
    if ((res = paging_v_addr_to_paging_indexes(virt_addr, &directory_index, &table_index)) < 0) {
        return res;
    }

    const uintptr_t page_directory_entry = handle->directory[directory_index];
    // extract Page Table addr from page_directory_entry
    uintptr_t *const page_table = (uintptr_t *)(page_directory_entry & ADDR_MASK);
    // set the Page table entry with the target addr its flags and all that
    page_table[table_index] = (uintptr_t)page_ptr | flags;

    return 0;
}

int paging_map_range(struct page_directory_handle *const directory, void *virt_addr, void *phys_addr, int count, int flags)
{
    if (!paging_addr_is_page_aligned(virt_addr) || !paging_addr_is_page_aligned(phys_addr)) {
        return -EINVAL;
    }

    for (int i = 0; i < count; ++i) {
        int res = paging_map_page(directory, virt_addr, phys_addr, flags);

        if (res) {
            return res;
        }

        virt_addr += PAGING_PAGE_SIZE;
        phys_addr += PAGING_PAGE_SIZE;
    }

    return 0;
}

int paging_map_from_to(struct page_directory_handle *const directory, void *virt_addr, void *phys_begin, void *phys_end, int flags)
{
    if (!paging_addr_is_page_aligned(virt_addr) || !paging_addr_is_page_aligned(phys_begin)) {
        return -EINVAL;
    }

    if (!paging_addr_is_page_aligned(phys_end)) {
        return -EINVAL;
    }

    if (phys_end < phys_begin) {
        return -EINVAL;
    }

    const int total_pages = (phys_end - phys_begin) / PAGING_PAGE_SIZE;

    return paging_map_range(directory, virt_addr, phys_begin, total_pages, flags);
}
