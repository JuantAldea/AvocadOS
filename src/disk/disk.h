#ifndef __DISK_h
#define __DISK_h
#include <stdint.h>

#define BUSY_BIT 0x80
#define DRQ_BIT  0x08
typedef uint8_t DISK_TYPE;
#define DISK_TYPE_PHYSICAL 0
#define DISK_SECTOR_SIZE 512
#define ATA_MASTER 0xE0
#define ATA_SLAVE 0xF0

struct disk
{
    DISK_TYPE type;
    int sector_size;
    uint8_t port;
};

int disk_read_block(const struct disk * const disk, const unsigned int lba, const int n, void * const buffer);
void disk_discover_and_init();
struct disk* disk_get(int disk_index);

#endif