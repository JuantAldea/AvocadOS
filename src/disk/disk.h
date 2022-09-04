#ifndef __DISK_h
#define __DISK_h
#include <stdint.h>

// clang-format off

#define BUSY_BIT 0x80
#define DRQ_BIT  0x08
#define ERR_BIT  0x01
#define DF_BIT  0x20
#define DISK_TYPE_PHYSICAL 0
#define DISK_SECTOR_SIZE 512
#define ATA_MASTER 0xE0
#define ATA_SLAVE 0xF0
#define ATA_PRIMARY_DEVICE_CONTROL 0x3F6
#define ATA_PRIMARY_ALTERNATE_STATUS ATA_PRIMARY_DEVICE_CONTROL
#define ATA_PRIMARY_STATUS 0x1F7

#define ATA_COMMAND_CACHE_FLUSH ((uint8_t)0xE7)
#define ATA_COMMAND_READ_SECTORS ((uint8_t)0x20)

#define ATA_PRIMARY_IO_BASE_REGISTER ((uint16_t) 0x1F0)
#define ATA_PRIMARY_CONTROL_BASE_REGISTER ((uint16_t) 0x3F6)
// clang-format on

struct filesystem_operations_t;

struct disk_t {
    uint8_t type;
    int sector_size;
    uint8_t master_slave_flag;
    int id;
    struct filesystem_operations_t *fs_operations;
    void *fs_private;
    const struct ata_ports_t *ata_ports;
};

struct ata_ports_t {
    uint16_t data_register;
    uint16_t features_error_register;
    uint16_t sector_count_register;
    uint16_t sector_number_register;
    uint16_t cylinder_low_register;
    uint16_t cylinder_high_register;
    uint16_t drive_head_register;
    uint16_t status_command_register;
    uint16_t device_control_alternate_status_register;
    uint16_t drive_address_register;
};

int disk_read_block(const struct disk_t *const disk, const unsigned int lba, const int n, void *const buffer);
void disk_init();
int disk_get(int disk_index, struct disk_t **disk);

#endif
