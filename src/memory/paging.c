#include "paging.h"
#include "kheap.h"
#include "../status.h"
#include "../config.h"
#include "../kernel/panic.h"
#include "../string/string.h"
#include "../termio/termio.h"

extern uintptr_t BOOT_PAGE_DIRECTORY;
extern struct page_directory_handle kernel_page_directory;
struct page_directory_handle *current_directory;

uint8_t *bitmap_begin;
uint8_t *bitmap_end;

extern void paging_load_directory(uintptr_t directory);

uintptr_t virtual_to_physical_addr(struct page_directory_handle *handle, void *virtual_addr)
{
    uint32_t cr4;
    asm volatile("mov %%cr4, %0" : "=r"(cr4));

    uintptr_t page_directory_entry = handle->directory[V_ADDR_PAGEDIR_INDEX(virtual_addr)];

    if (PAGE_ENTRY_PSE_BIT(page_directory_entry)) {
        if (!(cr4 & PSE_BIT)) {
            panic("PSE directory entry found, but PSE is OFF");
        }

        return PAGE_ENTRY_ADDR(page_directory_entry) + V_ADDR_PAGEFRAME_INDEX_PSE(virtual_addr);
    }

    uintptr_t page_table_entry = handle->directory_virtual[V_ADDR_PAGEDIR_INDEX(virtual_addr)][V_ADDR_PAGETBL_INDEX(virtual_addr)];

    return PAGE_ENTRY_ADDR(page_table_entry) + V_ADDR_PAGEFRAME_INDEX(virtual_addr);
}

void init_page_bitmap(size_t number_of_pages)
{
    bitmap_begin = kzalloc(number_of_pages * sizeof(uint8_t));

    if (!paging_addr_is_page_aligned(bitmap_begin)) {
        panic("early_kmalloc: paging_addr_is_page_aligned: false ");
    }

    bitmap_end = bitmap_begin + number_of_pages * sizeof(uint8_t);

    // mark the space taken by the bitmap itself as used
    memset(bitmap_begin, 1, (number_of_pages * sizeof(uint8_t)) >> 12); //NOLINT
}

/*
struct page_directory_handle *paging_init_4gb_directory(const uint16_t flags)
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
*/

bool paging_addr_is_page_aligned(const void *const addr)
{
    return ((uintptr_t)addr & (PAGING_PAGE_SIZE - 1)) == 0;
}

uintptr_t paging_align_address(uintptr_t addr)
{
    return (PAGING_PAGE_SIZE * (((uintptr_t)addr + (PAGING_PAGE_SIZE - 1)) / PAGING_PAGE_SIZE));
}

void paging_switch_directory(struct page_directory_handle *const handle)
{
    paging_load_directory(PAGE_ENTRY_ADDR(handle->directory[1023]));
    current_directory = handle;
}

int paging_map_page(struct page_directory_handle *handle, uintptr_t virt_addr, uintptr_t page_ptr, const uint16_t flags)
{
    if (!paging_addr_is_page_aligned((void *)virt_addr) || !paging_addr_is_page_aligned((void *)page_ptr)) {
        return -EINVAL;
    }

    uintptr_t directory_index = V_ADDR_PAGEDIR_INDEX(virt_addr);
    uintptr_t table_index = V_ADDR_PAGETBL_INDEX(virt_addr);

    if (!handle->directory_virtual[directory_index]) {
        handle->directory_virtual[directory_index] = kzalloc(PAGING_PAGE_SIZE);
        uintptr_t page_table_pysical_addr = virtual_to_physical_addr(current_directory, handle->directory_virtual[directory_index]);
        handle->directory[directory_index] = page_table_pysical_addr | flags;
    }

    if (!handle->directory_virtual[directory_index]) {
        handle->directory_virtual[directory_index] = kzalloc(PAGING_PAGE_SIZE);
    }

    handle->directory_virtual[directory_index][table_index] = page_ptr | flags;

    char directory_ptr_str[100];
    char directory_index_str[100];
    char page_table_str[100];
    char table_index_str[100];
    char page_ptr_str[100];
    itoa(PAGE_ENTRY_ADDR(handle->directory[1023]), directory_ptr_str);
    itoa(directory_index, directory_index_str);
    itoa(handle->directory[directory_index], page_table_str);
    itoa(table_index, table_index_str);
    itoa(page_ptr, page_ptr_str);
    print("*(");
    print(directory_ptr_str);
    print(")[");
    print(directory_index_str);
    print("] -> (*");
    print(page_table_str);
    print(")[");
    print(table_index_str);
    print("] -> ");
    print(page_ptr_str);
    print("\n");
    return 0;
}

int paging_map_range(struct page_directory_handle *const handle, uintptr_t virt_addr, uintptr_t phys_addr, int count, int flags)
{
    if (!paging_addr_is_page_aligned((void *)virt_addr) || !paging_addr_is_page_aligned((void *)phys_addr)) {
        return -EINVAL;
    }

    for (int i = 0; i < count; ++i) {
        int res = paging_map_page(handle, virt_addr, phys_addr, flags);

        if (res) {
            return res;
        }

        virt_addr += PAGING_PAGE_SIZE;
        phys_addr += PAGING_PAGE_SIZE;
    }

    return 0;
}

int paging_map_from_to(struct page_directory_handle *const handle, uintptr_t virt_addr, uintptr_t phys_begin, uintptr_t phys_end, int flags)
{
    if (!paging_addr_is_page_aligned((void *)virt_addr) || !paging_addr_is_page_aligned((void *)phys_begin)) {
        return -EINVAL;
    }

    if (!paging_addr_is_page_aligned((void *)phys_end)) {
        return -EINVAL;
    }

    if (phys_end < phys_begin) {
        return -EINVAL;
    }

    const int total_pages = (phys_end - phys_begin) / PAGING_PAGE_SIZE;

    return paging_map_range(handle, virt_addr, phys_begin, total_pages, flags);
}

struct page_directory_handle *paging_clone_directory(struct page_directory_handle *source)
{
    struct page_directory_handle *copy = kzalloc(sizeof(struct page_directory_handle));
    copy->directory = kzalloc(PAGING_PAGE_SIZE);
    memset(copy->directory_virtual, 0, sizeof(copy->directory_virtual)); //NOLINT

    for (int i = 0; i < 1023; i++) {
        if (!source->directory[i]) {
            continue;
        }

        /* do not duplicate kernel pages */
        if (kernel_page_directory.directory[i] == source->directory[i]) {
            copy->directory[i] = kernel_page_directory.directory[i];
            copy->directory_virtual[i] = kernel_page_directory.directory_virtual[i];
            continue;
        }

        copy->directory_virtual[i] = kzalloc(PAGING_PAGE_SIZE);
        memcpy(copy->directory_virtual[i], source->directory_virtual[i], PAGING_PAGE_SIZE); //NOLINT
        copy->directory[i] = virtual_to_physical_addr(current_directory, copy->directory_virtual[i]);
        copy->directory[i] |= PAGE_ENTRY_FLAGS(source->directory[i]);
    }

    // map last PDE to the directory itself
    copy->directory_virtual[1023] = copy->directory;
    copy->directory[1023] = virtual_to_physical_addr(current_directory, copy->directory_virtual[1023]);
    copy->directory[1023] |= PAGING_WRITABLE_PAGE | PAGING_PRESENT;

    return copy;
}

void paging_init()
{
    size_t memory_size = KERNEL_HEAP_SIZE;
    uint32_t number_of_pages = (memory_size + PAGING_PAGE_SIZE - 1) / PAGING_PAGE_SIZE;
    init_page_bitmap(number_of_pages);

    current_directory = kzalloc(sizeof(struct page_directory_handle));
    current_directory->directory = &BOOT_PAGE_DIRECTORY;

    memset(&kernel_page_directory, 0, sizeof(struct page_directory_handle)); //NOLINT;
    kernel_page_directory.directory = kzalloc(PAGING_PAGE_SIZE);

    // map last PDT onto itself -> https://forum.osdev.org/viewtopic.php?p=152010
    kernel_page_directory.directory[1023] = virtual_to_physical_addr(current_directory, kernel_page_directory.directory);
    kernel_page_directory.directory[1023] |= PAGING_WRITABLE_PAGE | PAGING_PRESENT;

    kernel_page_directory.directory_virtual[1023] = kernel_page_directory.directory;

    if (virtual_to_physical_addr(current_directory, kernel_page_directory.directory) & ~ADDR_MASK) {
        //Here lie countless wasted hours. For they should not be forgotten.
        panic("Page directory is not 4K aligned");
    }

    // identity map the first 4 megabytes
    if (paging_map_from_to(&kernel_page_directory, 0x0, 0x0, 0x400000, PAGING_WRITABLE_PAGE | PAGING_PRESENT)) {
        panic("BAD ADDRESS");
    }

    // identity map first the 4 megabytes to kernel virtual address (higher half)
    if (paging_map_from_to(&kernel_page_directory, KERNEL_VIRTUAL_BASE, 0x0, 0x400000, PAGING_WRITABLE_PAGE | PAGING_PRESENT)) {
        panic("BAD ADDRESS");
    }

    asm volatile("cli");

    asm volatile("mov %0, %%cr3" ::"r"(PAGE_ENTRY_ADDR(kernel_page_directory.directory[1023])));

    uint32_t cr0, cr4;

    asm volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 &= ~PSE_BIT;
    asm volatile("mov %0, %%cr4" ::"r"(cr4));

    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" ::"r"(cr0));

    kfree(current_directory);

    current_directory = &kernel_page_directory;
}
