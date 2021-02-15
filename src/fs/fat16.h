#ifndef __FAT16_H__
#define __FAT16_H__

#include <stdint.h>

#define FAT16_SIGNATURE 0x29
#define FAT16_SYSTEM_ID "FAT16   "
#define FAT16_SYSTEM_ID_LEN 8


enum fopen_mode;
struct file_descriptor_t;
struct path_part;
struct disk_t;

struct fat_header {
    uint8_t jump_nop[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entries;
    uint16_t number_of_sectors;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t sectors_big;

    uint8_t drive_number;
    uint8_t win_nt_bit;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_id_string[11];
    uint8_t system_id_string[8];
} __attribute__((packed));

int fat16_probe(struct disk_t *disk);
void *fat16_open(struct disk_t *disk, struct path_part *path, enum fopen_mode mode);
int fat16_close(struct disk_t *disk);
int fat16_read(struct disk_t *disk);
int fat16_write(struct disk_t *disk);
int fat16_seek(struct disk_t *disk);
int fat16_stat(struct disk_t *disk);
int fat16_link(struct disk_t *disk);
int fat16_unlink(struct disk_t *disk);

extern struct filesystem_operations_t fat16_operations;

#endif
