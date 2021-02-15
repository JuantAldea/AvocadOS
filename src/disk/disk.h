#ifndef __DISK_h
#define __DISK_h
#include <stdint.h>

// clang-format off

#define BUSY_BIT 0x80
#define DRQ_BIT  0x08
#define DISK_TYPE_PHYSICAL 0
#define DISK_SECTOR_SIZE 512
#define ATA_MASTER 0xE0
#define ATA_SLAVE 0xF0

// clang-format on

struct filesystem_operations_t;

struct disk_t {
    uint8_t type;
    int sector_size;
    uint8_t port;
    struct filesystem_operations_t *fs_operations;
    int id;
};

int disk_read_block(const struct disk_t *const disk, const unsigned int lba, const int n, void *const buffer);
void disk_init();
struct disk_t *disk_get(int disk_index);

#endif
