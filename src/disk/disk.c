#include <stddef.h>

#include "disk.h"
#include "../string/string.h"
#include "../io/io.h"
#include "../status.h"
#include "../fs/file.h"

struct disk_t disks[1];

static const struct ata_ports_t ATA_PORTS_PRIMARY = {
    .data_register = ATA_PRIMARY_IO_BASE_REGISTER,
    .features_error_register = ATA_PRIMARY_IO_BASE_REGISTER + 1,
    .sector_count_register = ATA_PRIMARY_IO_BASE_REGISTER + 2,
    .sector_number_register = ATA_PRIMARY_IO_BASE_REGISTER + 3,
    .cylinder_low_register = ATA_PRIMARY_IO_BASE_REGISTER + 4,
    .cylinder_high_register = ATA_PRIMARY_IO_BASE_REGISTER + 5,
    .drive_head_register = ATA_PRIMARY_IO_BASE_REGISTER + 6,
    .status_command_register = ATA_PRIMARY_IO_BASE_REGISTER + 7,

    .device_control_alternate_status_register = ATA_PRIMARY_CONTROL_BASE_REGISTER,
    .drive_address_register = ATA_PRIMARY_CONTROL_BASE_REGISTER + 1,
};

int disk_read_sector(const struct disk_t *const disk, const int lba, const int n, void *const buf)
{
    // do not send interrupts. Is a waste of time. We are completely devoted to waste time polling.
    outb(disk->ata_ports->device_control_alternate_status_register, (uint8_t)0x2);
    outb(disk->ata_ports->drive_head_register, (lba >> 24) | disk->master_slave_flag);
    outb(disk->ata_ports->sector_count_register, n);
    outb(disk->ata_ports->sector_number_register, (uint8_t)(lba & 0xFF));
    outb(disk->ata_ports->cylinder_low_register, (uint8_t)(lba >> 8));
    outb(disk->ata_ports->cylinder_high_register, (uint8_t)(lba >> 16));
    outb(disk->ata_ports->status_command_register, ATA_COMMAND_READ_SECTORS);
    uint16_t *ptr = buf;
    uint8_t status;

    for (int block = 0; block < n; ++block) {
        do {
            status = insb(disk->ata_ports->status_command_register);
        } while ((status & BUSY_BIT) || !(status & DRQ_BIT));

        for (int i = 0; i < 256; ++i) {
            *ptr++ = insw(disk->ata_ports->data_register);
            /*
            //cache flush
            outb(disk->ata_ports->device_control_alternate_status_register, ATA_COMMAND_CACHE_FLUSH);
            do {
                status = insb(disk->ata_ports->device_control_alternate_status_register);
            } while (status & BUSY_BIT);
            */
        }
    }

    do {
        status = insb(disk->ata_ports->device_control_alternate_status_register);
    } while (status & BUSY_BIT);

    return 0;
}

void disk_init()
{
    memset(&disks, 0, sizeof(disks)); // NOLINT
    disks[0].type = DISK_TYPE_PHYSICAL;
    disks[0].sector_size = DISK_SECTOR_SIZE;
    disks[0].master_slave_flag = ATA_MASTER;
    disks[0].id = 0;
    disks[0].ata_ports = &ATA_PORTS_PRIMARY;
    // probing must be the last action
    disks[0].fs_operations = fs_probe_fs(&disks[0]);
}

int disk_get(int disk_index, struct disk_t **disk)
{
    if (disk_index != 0) {
        *disk = NULL;
        return -ENOMEDIUM;
    }

    *disk = &disks[disk_index];
    return 0;
}

int disk_read_block(const struct disk_t *const disk, const unsigned int lba, const int n, void *const buffer)
{
    if (disk != &disks[0]) {
        return -ENOMEDIUM;
    }

    return disk_read_sector(disk, lba, n, buffer);
}
