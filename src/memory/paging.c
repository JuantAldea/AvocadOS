#include "paging.h"
#include "kheap.h"
#include "../status.h"
#include "../config.h"
#include "../kernel/panic.h"


extern uintptr_t BOOT_PAGE_DIRECTORY;
extern struct page_directory_handle kernel_page_directory;
static uintptr_t *current_directory;


uint8_t *bitmap_begin;
uint8_t *bitmap_end;


//void *(*allocator)(size_t) = &early_kmalloc;
#include "../string/string.h"
#define V_ADDR_PAGEDIR_INDEX(vaddr) (((uintptr_t)vaddr) >> 22)
#define V_ADDR_PAGETBL_INDEX(vaddr) ((((uintptr_t)vaddr) >> 12) & 0x3ff)
#define V_ADDR_PAGEFRAME_INDEX(vaddr) (((uintptr_t)vaddr) & 0xfff)

#define V_ADDR_PAGEFRAME_INDEX_PSE(vaddr) (((uintptr_t)vaddr) & 0x3fffff)
#define PAGE_ENTRY_PSE_BIT(page_directory_entry) (((uintptr_t)page_directory_entry) & 0b100000)
#define PAGE_ENTRY_TO_ADDR(page_entry) ((physical_addr_t *)(((uintptr_t)page_entry) & ADDR_MASK))
#define PAGE_BITMAP_INDEX(page_addr) (((uintptr_t)page_addr - (uintptr_t)bitmap_begin) >> 12)

physical_addr_t *virtual_to_physical_addr(uintptr_t *directory, uintptr_t *virtual_addr)
{
    uint32_t cr4;
    asm volatile("mov %%cr4, %0" : "=r"(cr4));

    const uintptr_t page_directory_entry = (directory)[V_ADDR_PAGEDIR_INDEX(virtual_addr)];
    physical_addr_t *page_table_pointer = PAGE_ENTRY_TO_ADDR(page_directory_entry);

    if (PAGE_ENTRY_PSE_BIT(page_directory_entry)) {
        if (!(cr4 & 0x00000010)) {
            panic("PSE directory entry found, but PSE is OFF");
        }

        return (physical_addr_t *)((uintptr_t)page_table_pointer + V_ADDR_PAGEFRAME_INDEX_PSE(virtual_addr));
    }

    const uintptr_t page_table_entry = ((uintptr_t *)page_table_pointer)[V_ADDR_PAGETBL_INDEX(virtual_addr)];
    physical_addr_t *page_frame_pointer = PAGE_ENTRY_TO_ADDR(page_table_entry);

    return (physical_addr_t *)((uintptr_t)page_frame_pointer + V_ADDR_PAGEFRAME_INDEX(virtual_addr));
}


void init_page_bitmap(size_t number_of_pages)
{
    //bitmap_begin = (uint8_t *)early_kmalloc_current_pos;
    bitmap_begin = kzalloc(number_of_pages * sizeof(uint8_t));

    if (!paging_addr_is_page_aligned(bitmap_begin)) {
        panic("early_kmalloc: paging_addr_is_page_aligned: false ");
    }

    //early_kmalloc_current_pos += number_of_pages * sizeof(uint8_t);
    //bitmap_end = (uint8_t *)early_kmalloc_current_pos;
    bitmap_end = bitmap_begin + number_of_pages * sizeof(uint8_t);

    // mark all pages as unused -> kzalloc
    //memset(bitmap_begin, 0, number_of_pages * sizeof(uint8_t)); //NOLINT

    // mark the space taken by the bitmap itself as used
    memset(bitmap_begin, 1, (number_of_pages * sizeof(uint8_t)) >> 12); //NOLINT
}

extern void paging_load_directory(const uintptr_t *const directory);

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

bool paging_addr_is_page_aligned(const void *const addr)
{
    return ((uintptr_t)addr & (PAGING_PAGE_SIZE - 1)) == 0;
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

void paging_load_directory_ASM(const struct page_directory_handle *const handle)
{
    asm volatile("mov %0, %%cr3" ::"b"(handle->directory));
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
#include "../termio/termio.h"

int _paging_map_page(uintptr_t *const directory, const void *const virt_addr, const void *const page_ptr, const uint16_t flags)
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

    if (!directory[directory_index]) {
        uintptr_t *page_table = kzalloc(sizeof(uintptr_t) * PAGING_ENTRIES_PER_TABLE);
        physical_addr_t *page_table_pysical_addr = virtual_to_physical_addr(current_directory, page_table);
        directory[directory_index] = (uintptr_t)page_table_pysical_addr | 0x3;
    }

    const uintptr_t page_directory_entry = directory[directory_index];
    // extract Page Table addr from page_directory_entry
    uintptr_t *const page_table = (uintptr_t *)(page_directory_entry & ADDR_MASK);
    // set the Page table entry with the target addr its flags and all that
    page_table[table_index] = (uintptr_t)page_ptr | flags;
    char directory_ptr_str[100];
    char directory_index_str[100];
    char page_table_str[100];
    char table_index_str[100];
    char page_ptr_str[100];
    itoa((uintptr_t)directory, directory_ptr_str);
    itoa(directory_index, directory_index_str);
    itoa((uintptr_t)page_table, page_table_str);
    itoa(table_index, table_index_str);
    itoa((uintptr_t)page_ptr, page_ptr_str);
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

int paging_map_page(struct page_directory_handle *const handle, const void *const virt_addr, const void *const page_ptr,
                    const uint16_t flags)
{
    return _paging_map_page(handle->directory, virt_addr, page_ptr, flags);
}

int _paging_map_range(uintptr_t *const directory, void *virt_addr, void *phys_addr, int count, int flags)
{
    if (!paging_addr_is_page_aligned(virt_addr) || !paging_addr_is_page_aligned(phys_addr)) {
        return -EINVAL;
    }

    for (int i = 0; i < count; ++i) {
        int res = _paging_map_page(directory, virt_addr, phys_addr, flags);

        if (res) {
            return res;
        }

        virt_addr += PAGING_PAGE_SIZE;
        phys_addr += PAGING_PAGE_SIZE;
    }

    return 0;
}

int paging_map_range(struct page_directory_handle *const directory, void *virt_addr, void *phys_addr, int count, int flags)
{
    return _paging_map_range(directory->directory, virt_addr, phys_addr, count, flags);
}

int _paging_map_from_to(uintptr_t *const directory, void *virt_addr, void *phys_begin, void *phys_end, int flags)
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

    return _paging_map_range(directory, virt_addr, phys_begin, total_pages, flags);
}

int paging_map_from_to(struct page_directory_handle *const directory, void *virt_addr, void *phys_begin, void *phys_end, int flags)
{
    return _paging_map_from_to(directory->directory, virt_addr, phys_begin, phys_end, flags);
}

void paging_init(size_t memory)
{
    current_directory = &BOOT_PAGE_DIRECTORY;
    //early_kmalloc_current_pos = (uintptr_t)&_kernel_end;

    uint32_t number_of_pages = (memory + PAGING_PAGE_SIZE - 1) / PAGING_PAGE_SIZE;

    init_page_bitmap(number_of_pages);

    uintptr_t *directory = kzalloc(sizeof(uintptr_t) * PAGING_ENTRIES_PER_TABLE);

    // map last PDT onto itself -> https://forum.osdev.org/viewtopic.php?p=152010
    directory[1023] = (uintptr_t)virtual_to_physical_addr(current_directory, directory) | 0xFFF;

    // identity map first 4 megabytes
    if (_paging_map_from_to((void *)directory, (void *)0x0, (void *)0x0, (void *)0x400000, 0x3)) {
        panic("BAD ADDRESS");
    }

    // identity map first 4 megabytes to kernel virtual address
    if (_paging_map_from_to((void *)directory, (void *)KERNEL_VIRTUAL_BASE, (void *)0x0, (void *)0x400000, 0x3)) {
        panic("BAD ADDRESS");
    }

    asm volatile("cli");

    asm volatile("mov %0, %%cr3" ::"r"(PAGE_ENTRY_TO_ADDR(directory[1023])));

    uint32_t cr0, cr4;

    asm volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 ^= 0x00000010;
    asm volatile("mov %0, %%cr4" ::"r"(cr4));

    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" ::"r"(cr0));

    current_directory = (void *)directory;
    kernel_page_directory.directory = directory;
    //paging_load_directory((void *)virtual_to_physical_addr(current_directory, directory));
    //enable_paging();
}
