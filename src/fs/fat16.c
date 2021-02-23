#include "fat16.h"
#include "../disk/disk_stream.h"
#include "../status.h"
#include "../string/string.h"
#include "../termio/termio.h"
#include "../disk/disk.h"
#include "path_parser.h"
#include "file.h"
#include "../memory/kheap.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

struct filesystem_operations_t fat16_operations = {
    .probe = fat16_probe,
    .open = fat16_open,
    .close = fat16_close,

    .read = fat16_read,
    /*
    .write = fat16_write,
    .seek = fat16_seek,
    .stat = fat16_stat,
    .unlink = fat16_unlink,
    */
};

void fat16_split_entry_name(char *name, char filename_out[8], char extension_out[3])
{
    size_t len = strnlen(name, 12);
    memset(filename_out, ' ', 8);//NOLINT
    memset(extension_out, ' ', 3); //NOLINT

    char *point_pointer = memchr(name, '.', len);
    if (!point_pointer) {
        memcpy(filename_out, name, len); //NOLINT
        return;
    }

    memcpy(filename_out, name, point_pointer - name); //NOLINT
    memcpy(extension_out, point_pointer + 1, (name + len) - point_pointer - 1); //NOLINT
}

uint32_t fat16_cluster_number_to_data_address(const struct fat_private_data_t *private, uint16_t cluster)
{
    return fat16_cluster_number_to_data_sector(private, cluster) * private->header.bytes_per_sector;
}

uint32_t fat16_cluster_number_to_data_sector(const struct fat_private_data_t *private, uint16_t cluster)
{
    return private->data_sector + (cluster - 2) * private->header.sectors_per_cluster;
}

uint16_t fat16_sector_to_cluster_number(const struct fat_private_data_t *private, uint32_t sector)
{
    return 2 + (sector - private->data_sector) / private->header.sectors_per_cluster;
}

uint16_t fat16_fat_next_cluster_number(const struct fat_private_data_t *private, uint16_t cluster)
{
    if (cluster == FAT16_CLUSTER_CHAIN_END) {
        return FAT16_CLUSTER_CHAIN_END;
    }

    const uint32_t cluster_fat_byte_offset = cluster * sizeof(cluster);
    uint32_t cluster_address = private->header.reserved_sectors * private->header.bytes_per_sector + cluster_fat_byte_offset;

    diskstream_seek(private->dev, cluster_address);
    uint16_t next_cluster;
    diskstream_read(private->dev, &next_cluster, sizeof(next_cluster));
    return next_cluster;
}

uint16_t fat16_cluster_chain_nth(const struct fat_private_data_t *private, uint16_t cluster, size_t nth)
{
    uint16_t next_cluster = cluster;

    for (size_t i = 0; i < nth && next_cluster != FAT16_CLUSTER_CHAIN_END; ++i) {
        next_cluster = fat16_fat_next_cluster_number(private, next_cluster);
    }

    return next_cluster;
}

void __fat16_advance_cursors_root_dir(struct fat_descriptor_t *descriptor, uint32_t amount)
{
    const struct fat_private_data_t *private = descriptor->disk->fs_private;

    descriptor->pos += amount;

    if (descriptor->pos == private->data_sector) {
        descriptor->eof = 1;
    }

    descriptor->address += amount;
}

void __fat16_advance_cursors_non_root(struct fat_descriptor_t *descriptor, uint32_t amount)
{
    const struct fat_private_data_t *private = descriptor->disk->fs_private;
    const struct fat_header_t *header = &private->header;

    uint32_t current_cluster_count = descriptor->pos / private->bytes_per_cluster;
    uint32_t current_cluster_number = fat16_sector_to_cluster_number(private, descriptor->current_cluster_first_sector);

    descriptor->pos += amount;

    descriptor->offset_in_cluster = descriptor->pos % private->bytes_per_cluster;
    uint32_t next_cluster_count = descriptor->pos / private->bytes_per_cluster;
    uint32_t next_cluster_number = fat16_cluster_chain_nth(private, current_cluster_number, next_cluster_count - current_cluster_count);

    if (next_cluster_number != FAT16_CLUSTER_CHAIN_END) {
        descriptor->current_cluster_first_sector = fat16_cluster_number_to_data_sector(private, next_cluster_number);
    } else {
        descriptor->eof = 1;
    }

    uint32_t data_region_addr = descriptor->current_cluster_first_sector * header->bytes_per_sector;
    descriptor->address = data_region_addr + descriptor->offset_in_cluster;
}

void fat16_advance_cursors(struct fat_descriptor_t *descriptor, uint32_t amount)
{
    if (descriptor->eof) {
        return;
    }

    const struct fat_private_data_t *private = descriptor->disk->fs_private;

    if (fat16_item_is_root_dir(private, &descriptor->item)) {
        __fat16_advance_cursors_root_dir(descriptor, amount);
    } else {
        __fat16_advance_cursors_non_root(descriptor, amount);
    }
}

struct fat_item_t *traverse_path(struct disk_t *disk, struct path_part *path)
{
    struct fat_private_data_t *private = disk->fs_private;

    struct fat_item_t *item = kzalloc(sizeof(struct fat_item_t));
    memcpy(item, &private->root_directory, sizeof(struct fat_item_t)); //NOLINT

    while (path) {
        struct fat_descriptor_t *folder_descriptor = fat16_opendir(disk, item);

        kfree(item);

        if (!folder_descriptor) {
            return NULL;
        }
        char filename[8];
        char extension[3];
        fat16_split_entry_name(path->part, filename, extension);
        item = fat16_find_entry_in_dir(folder_descriptor, filename, extension);

        kfree(folder_descriptor);

        if (!item) {
            return NULL;
        }

        path = path->next;
    }

    return item;
}

void fat16_rewind_cursors(struct fat_descriptor_t *descriptor)
{
    const struct fat_private_data_t *private = descriptor->disk->fs_private;
    const struct fat_header_t *header = &private->header;
    descriptor->pos = 0;
    descriptor->offset_in_cluster = 0;
    descriptor->eof = 0;

    if (!fat16_item_is_root_dir(private, &descriptor->item)) {
        descriptor->current_cluster_first_sector = descriptor->item.first_sector;
    }

    descriptor->address = descriptor->current_cluster_first_sector * header->bytes_per_sector;
}

int fat16_count_dir_items(struct fat_descriptor_t *descriptor)
{
    fat16_rewind_cursors(descriptor);
    int count = 0;
    while (!descriptor->eof) {
        struct fat_entry_t entry;
        int res = __fat16_read(descriptor, &entry, sizeof(struct fat_entry_t));
        if (res < 0) {
            return -EIO;
        }

        if (entry.attributes == FAT16_FILE_LFN || entry.attributes & FAT16_FILE_VOLUME_LABEL) {
            continue;
        }

        if (!entry.filename[0]) {
            descriptor->eof = 1;
            break;
        }

        ++count;
    }

    fat16_rewind_cursors(descriptor);

    return count;
}

int fat16_item_is_root_dir(const struct fat_private_data_t *const private, const struct fat_item_t *const dir)
{
    if (dir->type != FAT16_ROOT_DIRECTORY_FLAG) {
        return 0;
    }

    return dir->first_sector == private->root_directory.first_sector;
}

int read_sector(struct disk_t *disk, uint32_t sector, unsigned int *buffer)
{
    const struct fat_private_data_t *private = disk->fs_private;
    const struct fat_header_t *header = &private->header;
    const uint32_t address = sector * header->bytes_per_sector;
    diskstream_seek(private->dev, address);
    int to_read = header->bytes_per_sector;
    int read = 0;
    while (read < to_read) {
        int chunk = diskstream_read(private->dev, buffer + read, to_read - read);
        if (chunk <= 0) {
            return chunk;
        }
        read += chunk;
    }
    return read;
}

struct fat_item_t *fat16_find_entry_in_dir(struct fat_descriptor_t *descriptor, const char *filename, const char *extension)
{
    struct fat_private_data_t *private = descriptor->disk->fs_private;

    if (descriptor->item.type != FAT16_DIRECTORY_FLAG && descriptor->item.type != FAT16_ROOT_DIRECTORY_FLAG) {
        return NULL;
    }

    size_t filename_len = strnlen(filename, 8);
    size_t extension_len = strnlen(extension, 3);

    char padded_filename[8];
    char padded_extension[3];

    memset(&padded_filename, ' ', sizeof(padded_filename)); //NOLINT
    memset(&padded_extension, ' ', sizeof(padded_extension)); //NOLINT

    memcpy(&padded_filename, filename, filename_len); //NOLINT
    memcpy(&padded_extension, extension, extension_len); //NOLINT

    struct fat_entry_t entry = { 0 };

    uint32_t dir_pos = 0;
    fat16_rewind_cursors(descriptor);
    while (!descriptor->eof) {
        int res = __fat16_read(descriptor, &entry, sizeof(struct fat_entry_t));

        ++dir_pos;

        if (res < 0) {
            goto out;
        }

        if (entry.attributes == FAT16_FILE_LFN || entry.attributes & FAT16_FILE_VOLUME_LABEL) {
            continue;
        }

        if (!entry.filename[0]) {
            descriptor->eof = 1;
            break;
        }

        if (strncmp(padded_filename, entry.filename, 8) || strncmp(padded_extension, entry.extension, 3)) {
            continue;
        }

        struct fat_item_t *item = kzalloc(sizeof(struct fat_item_t));

        if (!item) {
            return NULL;
        }

        item->type = FAT16_FILE_SUBDIRECTORY & entry.attributes ? FAT16_DIRECTORY_FLAG : FAT16_FILE_FLAG;
        item->first_sector =
                entry.low16_bits_first_cluster ? fat16_cluster_number_to_data_sector(private, entry.low16_bits_first_cluster) : 0;
        item->parent_dir_first_sector = descriptor->item.first_sector;
        item->parent_dir_position = dir_pos;
        item->entry = entry;

        return item;
    }

out:
    fat16_rewind_cursors(descriptor);
    return NULL;
}

size_t fat16_read_file(struct fat_descriptor_t *descriptor, void *ptr, size_t len)
{
    if (descriptor->eof) {
        return 0;
    }

    len = MIN(len, descriptor->item.entry.size_bytes - descriptor->pos);
    int res = __fat16_read(descriptor, ptr, len);

    if (descriptor->pos >= descriptor->item.entry.size_bytes) {
        descriptor->eof = 1;
    }

    return res;
}

size_t __fat16_read(struct fat_descriptor_t *descriptor, void *ptr, size_t len)
{
    struct fat_private_data_t *private = descriptor->disk->fs_private;

    size_t read = 0;
    while (read < len && !descriptor->eof) {
        diskstream_seek(private->dev, descriptor->address);
        const size_t to_read_in_cluster = MIN(len - read, private->bytes_per_cluster - descriptor->offset_in_cluster);
        const int chunk_read = diskstream_read(private->dev, ptr, to_read_in_cluster);

        if (chunk_read <= 0) {
            return chunk_read;
        }

        read += chunk_read;
        ptr += chunk_read;
        fat16_advance_cursors(descriptor, chunk_read);
    }

    return read;
}

void fat16_fat_private_free(struct fat_private_data_t **ptr)
{
    struct fat_private_data_t *fat_private = *ptr;

    if (fat_private && fat_private->dev) {
        kfree(fat_private->dev);
    }

    if (fat_private) {
        kfree(fat_private);
    }

    *ptr = NULL;
}

int fat16_probe(struct disk_t *disk)
{
    struct disk_stream *stream = diskstream_new(disk->id);

    if (!stream) {
        return -ENOMEM;
    }

    struct fat_header_t header = { 0 };

    int res = diskstream_read(stream, &header, sizeof(struct fat_header_t));
    diskstream_close(stream);

    if (res < 0) {
        return -EIO;
    }

    res = 0;

    if (header.signature != FAT16_SIGNATURE) {
        return -EMEDIUMTYPE;
    }

    if (strncmp((char *)header.system_id_string, FAT16_SYSTEM_ID, FAT16_SYSTEM_ID_LEN)) {
        return -EMEDIUMTYPE;
    }

    struct fat_private_data_t *fat_private = kzalloc(sizeof(struct fat_private_data_t));

    if (!fat_private) {
        return -ENOMEM;
    }

    fat_private->root_directory.first_sector = header.reserved_sectors + (header.fat_copies * header.sectors_per_fat);
    fat_private->root_directory.type = FAT16_ROOT_DIRECTORY_FLAG;

    // rounding paraphernalia
    const uint32_t root_dir_sector_len =
            (header.root_dir_entries * sizeof(struct fat_entry_t) + header.bytes_per_sector - 1) / header.bytes_per_sector;

    fat_private->data_sector = fat_private->root_directory.first_sector + root_dir_sector_len;

    fat_private->dev = diskstream_new(disk->id);
    fat_private->bytes_per_cluster = header.bytes_per_sector * header.sectors_per_cluster;

    if (!fat_private->dev) {
        res = -ENOMEM;
        goto out;
    }

    fat_private->header = header;

    disk->fs_private = fat_private;

out:
    if (res) {
        fat16_fat_private_free(&fat_private);
    }

    return res;
}

struct fat_descriptor_t *fat16_opendir(struct disk_t *disk, struct fat_item_t *item)
{
    struct fat_descriptor_t *descriptor = __fat16_opendir(disk, item->first_sector);

    if (descriptor) {
        descriptor->item = *item;
    }

    return descriptor;
}

struct fat_descriptor_t *__fat16_opendir(struct disk_t *disk, uint32_t sector)
{
    struct fat_private_data_t *fat_private = (struct fat_private_data_t *)disk->fs_private;
    struct fat_header_t *header = &fat_private->header;
    struct fat_descriptor_t *descriptor = kzalloc(sizeof(struct fat_descriptor_t));

    if (!descriptor) {
        return NULL;
    }

    descriptor->disk = disk;
    descriptor->first_cluster = (sector == fat_private->root_directory.first_sector) ? FAT16_CLUSTER_CHAIN_END :
                                                                                       fat16_sector_to_cluster_number(fat_private, sector);
    descriptor->current_cluster_first_sector = sector;
    descriptor->address = descriptor->current_cluster_first_sector * header->bytes_per_sector;
    descriptor->item.type = FAT16_DIRECTORY_FLAG;

    return descriptor;
}

struct fat_descriptor_t *fat16_open_item(struct disk_t *disk, struct fat_item_t *item)
{
    struct fat_descriptor_t *descriptor = kzalloc(sizeof(struct fat_descriptor_t));

    if (!descriptor) {
        return NULL;
    }

    descriptor->disk = disk;
    descriptor->item = *item;
    descriptor->item.type = item->entry.attributes & FAT16_FILE_SUBDIRECTORY ? FAT16_DIRECTORY_FLAG : FAT16_FILE_FLAG;
    descriptor->first_cluster = item->entry.low16_bits_first_cluster;
    fat16_rewind_cursors(descriptor);
    return descriptor;
}

void *fat16_open(struct disk_t *disk, struct path_part *path, enum fopen_mode mode)
{
    (void)mode;

    struct fat_item_t *item = traverse_path(disk, path);

    if (!item) {
        return NULL;
    }

    struct fat_descriptor_t *descriptor = fat16_open_item(disk, item);

    kfree(item);

    return descriptor;
}

int fat16_close(void *priv)
{
    //struct fat_descriptor_t *d = priv;
    kfree(priv);
    return 0;
}

size_t fat16_read(struct disk_t *disk, void *priv, uint32_t size, uint32_t nmemb, char *out)
{
    (void)disk;
    //struct fat_descriptor_t *d = priv;
    return fat16_read_file(priv, out, size * nmemb);
}

int fat16_write(struct disk_t *disk)
{
    (void)disk;
    return 0;
}

int fat16_seek(struct disk_t *disk)
{
    (void)disk;
    return 0;
}

int fat16_stat(struct disk_t *disk)
{
    (void)disk;
    return 0;
}

int fat16_link(struct disk_t *disk)
{
    (void)disk;
    return 0;
}

int fat16_unlink(struct disk_t *disk)
{
    (void)disk;
    return 0;
}
