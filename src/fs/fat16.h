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
#define FAT16_CLUSTER_CHAIN_END 0xFFF8

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
    uint16_t first_cluster;
    int eof;
    uint32_t pos;
    uint32_t offset_in_cluster;
    uint32_t current_cluster_first_sector;
    uint32_t address;
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
size_t fat16_read(struct disk_t *disk, void *priv, uint32_t size, uint32_t nmemb, char *out);
int fat16_write(struct disk_t *disk);
int fat16_seek(struct disk_t *disk);
int fat16_stat(struct disk_t *disk);
int fat16_link(struct disk_t *disk);
int fat16_unlink(struct disk_t *disk);

struct fat_item_t *fat16_find_entry_in_dir(struct fat_descriptor_t *descriptor, const char *filename, const char *extension);
extern struct filesystem_operations_t fat16_operations;

uint16_t fat16_fat_next_cluster_number(const struct fat_private_data_t *private, uint16_t cluster);

uint16_t fat16_cluster_chain_nth(const struct fat_private_data_t *private, uint16_t cluster, size_t nth);
size_t __fat16_read(struct fat_descriptor_t *descriptor, void *ptr, size_t len);
struct fat_descriptor_t *fat16_open2(struct disk_t *disk, struct fat_entry_t *entry);

//struct fat_descriptor_t *create_root_dir_descriptor(struct disk_t *disk);
int fat16_read_dir(struct fat_descriptor_t *dir, struct fat_entry_t *entry);
struct fat_descriptor_t *__fat16_opendir(struct disk_t *disk, uint32_t sector);
struct fat_descriptor_t *fat16_opendir(struct disk_t *disk, struct fat_item_t *item);

struct fat_item_t *traverse_path(struct disk_t *disk, struct path_part *path);
void fat16_fat_private_free(struct fat_private_data_t **ptr);

int fat16_testing(struct disk_t *disk);

uint16_t fat16_sector_to_cluster_number(const struct fat_private_data_t *private, uint32_t sector);
uint32_t fat16_cluster_number_to_data_sector(const struct fat_private_data_t *private, uint16_t cluster);
uint32_t fat16_cluster_number_to_data_address(const struct fat_private_data_t *private, uint16_t cluster);
size_t fat16_read_file(struct fat_descriptor_t *descriptor, void *ptr, size_t len);

int fat16_item_is_root_dir(const struct fat_private_data_t *const private, const struct fat_item_t *const dir);

#endif
