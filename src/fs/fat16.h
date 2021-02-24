#ifndef __FAT16_H__
#define __FAT16_H__

#include <stdint.h>
#include <stddef.h>

#define FAT16_SIGNATURE 0x29
#define FAT16_SYSTEM_ID "FAT16   "
#define FAT16_SYSTEM_ID_LEN 8

typedef uint8_t FAT_ENTRY_TYPE;

#define FAT16_DIRECTORY_FLAG 0
#define FAT16_FILE_FLAG 1
#define FAT16_ROOT_DIRECTORY_FLAG 255

#define FAT16_FILE_READ_ONLY 0x01
#define FAT16_FILE_HIDDEN 0x02
#define FAT16_FILE_SYSTEM 0x04
#define FAT16_FILE_VOLUME_LABEL 0x8
#define FAT16_FILE_SUBDIRECTORY 0x10
#define FAT16_FILE_ARCHIVED 0x20
#define FAT16_FILE_DEVICE 0x40
#define FAT16_FILE_RESERVED 0x80
#define FAT16_FILE_LFN 0x0F

#define FAT16_CLUSTER_RESERVED 0xFFF6
#define FAT16_CLUSTER_BAD_OR_RESERVED 0xFFF7
#define FAT16_CLUSTER_CHAIN_END_BEGIN 0xFFF8
#define FAT16_CLUSTER_CHAIN_END_END 0xFFFF

extern struct filesystem_operations_t fat16_operations;

enum fopen_mode;
struct file_descriptor_t;
struct path_part;
struct disk_t;

struct fat_header_t {
    uint8_t jump_nop[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entries;
    uint16_t total_sectors;
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

struct fat_entry_t {
    char filename[8];
    char extension[3];
    uint8_t attributes;
    uint8_t reserved_win_nt;
    uint8_t creation_time_tenths_of_sec;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access;
    uint16_t high16_bits_first_cluster;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t low16_bits_first_cluster;
    uint32_t size_bytes;

} __attribute__((packed));

struct fat_item_t {
    FAT_ENTRY_TYPE type;
    struct fat_entry_t entry;
    uint32_t first_sector;
    uint32_t parent_dir_first_sector;
    uint32_t parent_dir_position;
};

struct fat_descriptor_t {
    struct fat_item_t item;
    int eof;
    uint32_t pos;
    uint32_t offset_in_cluster;
    uint32_t current_cluster_first_sector;
    struct disk_t *disk;
};

struct fat_private_data_t {
    struct fat_header_t header;
    uint32_t bytes_per_cluster;
    uint32_t data_sector;
    struct fat_item_t root_directory;
    struct disk_stream *dev;
};



int fat16_probe(struct disk_t *disk);
void *fat16_open(struct disk_t *disk, struct path_part *path, enum fopen_mode mode);
int fat16_close(void *priv);
size_t fat16_read(void *priv, uint32_t size, uint32_t nmemb, char *out);
int fat16_seek(void *priv, int32_t offset, int whence);

int fat16_write(struct disk_t *disk);
int fat16_stat(struct disk_t *disk);
int fat16_unlink(struct disk_t *disk);

struct fat_item_t *fat16_find_entry_in_dir(struct fat_descriptor_t *descriptor, const char *filename, const char *extension);
struct fat_item_t *traverse_path(struct disk_t *disk, struct path_part *path);

#endif
