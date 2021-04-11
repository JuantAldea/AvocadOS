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
    .stat = fat16_stat,

    /*
    .write = fat16_write,
    .seek = fat16_seek,
    .unlink = fat16_unlink,
    */
};

size_t __fat16_read(struct fat_descriptor_t *descriptor, void *ptr, size_t len);
int __fat16_adjust_cursors_root_dir(struct fat_descriptor_t *descriptor);
int __fat16_adjust_cursors_non_root(struct fat_descriptor_t *descriptor);
int __fat16_adjust_cursors(struct fat_descriptor_t *descriptor, int32_t offset);
uint32_t __fat16_descriptor_to_absolute_address(struct fat_descriptor_t *descriptor);
void __fat16_reset_cursors(struct fat_descriptor_t *descriptor);
int __fat16_seek_set(struct fat_descriptor_t *descriptor, int32_t pos);
struct fat_descriptor_t *__fat16_open(struct disk_t *disk, struct fat_item_t *item);

uint16_t fat16_sector_to_cluster_number(const struct fat_private_data_t *private, uint32_t sector);
uint32_t fat16_cluster_number_to_data_sector(const struct fat_private_data_t *private, uint16_t cluster);
uint32_t fat16_cluster_number_to_data_address(const struct fat_private_data_t *private, uint16_t cluster);
int fat16_item_is_root_dir(const struct fat_private_data_t *const private, const struct fat_item_t *const dir);
void fat16_fat_private_free(struct fat_private_data_t **ptr);

void fat16_split_entry_name(char *name, char filename_out[8], char extension_out[3])
{
    size_t len = strnlen(name, 12);
    memset(filename_out, ' ', 8); //NOLINT
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
    if (cluster < 2) {
        return 0;
    }

    return private->data_sector + (cluster - 2) * private->header.sectors_per_cluster;
}

uint16_t fat16_sector_to_cluster_number(const struct fat_private_data_t *private, uint32_t sector)
{
    return 2 + (sector - private->data_sector) / private->header.sectors_per_cluster;
}

uint16_t fat16_read_fat(const struct fat_private_data_t *private, uint16_t cluster)
{
    if (cluster >= FAT16_CLUSTER_CHAIN_END_BEGIN) {
        return FAT16_CLUSTER_CHAIN_END_BEGIN;
    }

    const uint32_t cluster_fat_byte_offset = cluster * sizeof(cluster);
    uint32_t cluster_address = private->header.reserved_sectors * private->header.bytes_per_sector + cluster_fat_byte_offset;

    uint16_t next_cluster;
    diskstream_seek(private->dev, cluster_address);
    diskstream_read(private->dev, &next_cluster, sizeof(next_cluster));
    return next_cluster;
}

int fat16_write_fat(const struct fat_private_data_t *private, uint16_t cluster, uint16_t value)
{
    (void)private;
    (void)cluster;
    (void)value;
    return 0;
}

uint16_t fat16_get_free_cluster(const struct fat_private_data_t *private)
{
    (void)private;
    return FAT16_CLUSTER_CHAIN_END_BEGIN;
}

uint16_t fat16_follow_cluster_chain(const struct fat_private_data_t *private, uint16_t cluster, size_t steps)
{
    uint16_t next_cluster = cluster;

    for (size_t i = 0; i < steps && next_cluster < FAT16_CLUSTER_CHAIN_END_BEGIN; ++i) {
        next_cluster = fat16_read_fat(private, next_cluster);
        if (!next_cluster || next_cluster == FAT16_CLUSTER_RESERVED || next_cluster == FAT16_CLUSTER_BAD_OR_RESERVED) {
            break;
        }
    }

    return next_cluster;
}

int __fat16_adjust_cursors_root_dir(struct fat_descriptor_t *descriptor)
{
    const struct fat_private_data_t *private = descriptor->disk->fs_private;

    descriptor->relative_pos = descriptor->pos;

    if (descriptor->pos >= private->data_sector) {
        descriptor->eof = 1;
        if (descriptor->pos > private->data_sector) {
            descriptor->error = -EIO;
            return -EIO;
        }
    }

    return 0;
}

struct fat_cluster_chain {
    uint16_t first;
    uint16_t previous;
    uint16_t current;
    struct fat_private_data_t *private;
};

void fat16_cluster_chain_init(uint16_t begin, struct fat_private_data_t *private, struct fat_cluster_chain *chain)
{
    chain->first = begin;
    chain->previous = begin;
    chain->current = begin;
    chain->private = private;
}

void fat16_cluster_chain_next(struct fat_cluster_chain *chain)
{
    if (chain->current == FAT16_CLUSTER_BAD_OR_RESERVED || chain->current == FAT16_CLUSTER_RESERVED ||
        chain->current >= FAT16_CLUSTER_CHAIN_END_BEGIN) {
        return;
    }

    chain->previous = chain->current;
    chain->current = fat16_read_fat(chain->private, chain->current);
}

int fat16_cluster_chain_is_over(const struct fat_cluster_chain *const chain)
{
    return chain->current == FAT16_CLUSTER_CHAIN_END_BEGIN;
}

int fat16_cluster_chain_ok(const struct fat_cluster_chain *const chain)
{
    return chain->current != FAT16_CLUSTER_BAD_OR_RESERVED && chain->current != FAT16_CLUSTER_RESERVED;
}

int fat16_cluster_allocate_next(struct fat_cluster_chain *const chain)
{
    if (!fat16_cluster_chain_is_over(chain)) {
        return -EINVAL;
    }

    uint16_t free_cluster = fat16_get_free_cluster(chain->private);

    if (free_cluster >= FAT16_CLUSTER_CHAIN_END_BEGIN) {
        return -ENOSPC;
    }

    if (fat16_write_fat(chain->private, free_cluster, FAT16_CLUSTER_CHAIN_END_BEGIN)) {
        return -EIO;
    }

    if (fat16_write_fat(chain->private, chain->previous, free_cluster)) {
        return -EIO;
    }

    chain->current = free_cluster;

    return 0;
}

int __fat16_adjust_cursors_non_root(struct fat_descriptor_t *descriptor)
{
    const struct fat_private_data_t *private = descriptor->disk->fs_private;

    if (!descriptor->item.entry.low16_bits_first_cluster) {
        descriptor->eof = 1;
        return 0;
    }

    uint32_t cluster_count = descriptor->pos / private->bytes_per_cluster;
    uint16_t first_cluster = fat16_sector_to_cluster_number(private, descriptor->item.first_sector);
    uint16_t current_cluster = fat16_follow_cluster_chain(private, first_cluster, cluster_count);
    descriptor->relative_pos = descriptor->pos % private->bytes_per_cluster;

    if (current_cluster == FAT16_CLUSTER_RESERVED || current_cluster == FAT16_CLUSTER_BAD_OR_RESERVED) {
        descriptor->error = -EIO;
        return -EIO;
    }

    if (current_cluster < FAT16_CLUSTER_CHAIN_END_BEGIN) {
        descriptor->current_cluster_first_sector = fat16_cluster_number_to_data_sector(private, current_cluster);
    } else {
        descriptor->eof = 1;
    }

    return 0;
}

int __fat16_adjust_cursors(struct fat_descriptor_t *descriptor, int32_t offset)
{
    const struct fat_private_data_t *private = descriptor->disk->fs_private;

    descriptor->pos += offset;

    if (fat16_item_is_root_dir(private, &descriptor->item)) {
        return __fat16_adjust_cursors_root_dir(descriptor);
    }

    return __fat16_adjust_cursors_non_root(descriptor);
}

uint32_t __fat16_descriptor_to_absolute_address(struct fat_descriptor_t *descriptor)
{
    const struct fat_private_data_t *private = descriptor->disk->fs_private;
    const struct fat_header_t *header = &private->header;

    const uint32_t data_region_addr = descriptor->current_cluster_first_sector * header->bytes_per_sector;

    return data_region_addr + descriptor->relative_pos;
}

struct fat_item_t *traverse_path(struct disk_t *disk, struct path_part *path)
{
    struct fat_private_data_t *private = disk->fs_private;

    struct fat_item_t *item = kzalloc(sizeof(struct fat_item_t));
    memcpy(item, &private->root_directory, sizeof(struct fat_item_t)); //NOLINT

    for (; path; path = path->next) {
        struct fat_descriptor_t *folder_descriptor = __fat16_open(disk, item);

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
    }

    return item;
}

void __fat16_reset_cursors(struct fat_descriptor_t *descriptor)
{
    descriptor->pos = 0;
    descriptor->relative_pos = 0;
    descriptor->current_cluster_first_sector = descriptor->item.first_sector;
    descriptor->eof = 0;
}

int fat16_count_dir_items(struct fat_descriptor_t *descriptor)
{
    __fat16_reset_cursors(descriptor);
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

    __fat16_reset_cursors(descriptor);

    return count;
}

int fat16_item_is_root_dir(const struct fat_private_data_t *const private, const struct fat_item_t *const dir)
{
    if (dir->type != FAT16_ROOT_DIRECTORY_FLAG) {
        return 0;
    }

    return dir->first_sector == private->root_directory.first_sector;
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
    __fat16_reset_cursors(descriptor);
    struct fat_item_t *item = NULL;

    while (!descriptor->eof) {
        int res = __fat16_read(descriptor, &entry, sizeof(struct fat_entry_t));
        if (res < 0) {
            goto out;
        }

        ++dir_pos;

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

        item = kzalloc(sizeof(struct fat_item_t));

        if (!item) {
            return NULL;
        }

        item->type = FAT16_FILE_SUBDIRECTORY & entry.attributes ? FAT16_DIRECTORY_FLAG : FAT16_FILE_FLAG;
        item->first_sector = fat16_cluster_number_to_data_sector(private, entry.low16_bits_first_cluster);
        item->parent_dir_first_sector = descriptor->item.first_sector;
        item->parent_dir_position = dir_pos;
        item->entry = entry;

        break;
    }

out:
    __fat16_reset_cursors(descriptor);

    return item;
}

size_t fat16_read_file(struct fat_descriptor_t *descriptor, void *ptr, size_t len)
{
    int res = __fat16_adjust_cursors(descriptor, 0);
    if (res) {
        return res;
    }

    if (descriptor->eof) {
        return 0;
    }

    if (descriptor->pos >= descriptor->item.entry.size_bytes) {
        descriptor->eof = 1;
        return 0;
    }

    len = MIN(len, descriptor->item.entry.size_bytes - descriptor->pos);

    res = __fat16_read(descriptor, ptr, len);
    if (res) {
        return res;
    }

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
        diskstream_seek(private->dev, __fat16_descriptor_to_absolute_address(descriptor));
        const size_t to_read_sequentially = MIN(len - read, private->bytes_per_cluster - descriptor->relative_pos);
        const int chunk_read = diskstream_read(private->dev, ptr, to_read_sequentially);

        if (chunk_read <= 0) {
            descriptor->error = chunk_read;
            return chunk_read;
        }

        int res = __fat16_adjust_cursors(descriptor, chunk_read);
        if (res) {
            return res;
        }

        read += chunk_read;
        ptr = (char *)ptr + chunk_read;
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

    memset(&fat_private->root_directory, 0, sizeof(fat_private->root_directory)); //NOLINT
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

struct fat_descriptor_t *__fat16_open(struct disk_t *disk, struct fat_item_t *item)
{
    struct fat_private_data_t *fat_private = (struct fat_private_data_t *)disk->fs_private;
    struct fat_descriptor_t *descriptor = kzalloc(sizeof(struct fat_descriptor_t));

    if (!descriptor) {
        return NULL;
    }

    descriptor->disk = disk;
    descriptor->item = *item;

    __fat16_reset_cursors(descriptor);

    if (fat16_item_is_root_dir(fat_private, item)) {
        descriptor->item.type = FAT16_ROOT_DIRECTORY_FLAG;
    } else if (item->entry.attributes & FAT16_FILE_SUBDIRECTORY) {
        descriptor->item.type = FAT16_DIRECTORY_FLAG;
    } else {
        descriptor->item.type = FAT16_FILE_FLAG;
    }

    return descriptor;
}

void *fat16_open(struct disk_t *disk, struct path_part *path, enum fopen_mode mode)
{
    (void)mode;

    struct fat_item_t *item = traverse_path(disk, path);

    if (!item) {
        return NULL;
    }

    struct fat_descriptor_t *descriptor = __fat16_open(disk, item);

    kfree(item);

    return descriptor;
}

int fat16_close(void *priv)
{
    kfree(priv);
    return 0;
}

size_t fat16_read(void *priv, uint32_t size, uint32_t nmemb, char *out)
{
    struct fat_descriptor_t *descriptor = priv;
    int res = 0;

    if ((res = __fat16_adjust_cursors(descriptor, 0))) {
        return res;
    }

    if (descriptor->eof) {
        return 0;
    }

    if (descriptor->pos >= descriptor->item.entry.size_bytes) {
        descriptor->eof = 1;
        return 0;
    }

    size_t len = MIN(size * nmemb, descriptor->item.entry.size_bytes - descriptor->pos);
    res = __fat16_read(descriptor, out, len);

    if (descriptor->pos >= descriptor->item.entry.size_bytes) {
        descriptor->eof = 1;
    }

    return res;
}

int fat16_write(struct disk_t *disk)
{
    (void)disk;
    return 0;
}

int __fat16_seek_set(struct fat_descriptor_t *descriptor, int32_t pos)
{
    descriptor->pos = pos;
    descriptor->eof = 0;
    return 0;
}

int fat16_seek(void *priv, int32_t offset, int whence)
{
    struct fat_descriptor_t *descriptor = priv;

    switch (whence) {
    case SEEK_SET:
        return __fat16_seek_set(descriptor, offset);
    case SEEK_CUR:
        return __fat16_seek_set(descriptor, descriptor->pos + offset);
    case SEEK_END:
        return __fat16_seek_set(descriptor, descriptor->item.entry.size_bytes);
    }

    return 1;
}

int fat16_stat(void *priv, struct stat *buf)
{
    struct fat_descriptor_t *descriptor = priv;
    buf->st_dev = descriptor->disk->id;
    buf->st_size = descriptor->item.entry.size_bytes;
    return 0;
}

int fat16_unlink(struct disk_t *disk)
{
    (void)disk;
    return 0;
}
