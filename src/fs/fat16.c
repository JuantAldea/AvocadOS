#include "fat16.h"
#include "../disk/disk_stream.h"
#include "../status.h"
#include "../status.h"
#include "../string/string.h"
#include "../termio/termio.h"
#include "../disk/disk.h"
#include "path_parser.h"
#include "file.h"

struct filesystem_operations_t fat16_operations = {
    .probe = fat16_probe,
    .open = fat16_open,
    /*
    .close = fat16_close,
    .read = fat16_read,
    .write = fat16_write,
    .seek = fat16_seek,
    .stat = fat16_stat,
    .unlink = fat16_unlink,
    */
};

int fat16_probe(struct disk_t *disk)
{
    struct disk_stream *stream = diskstream_new(disk->id);

    struct fat_header header = { 0 };

    if (diskstream_read(stream, &header, sizeof(struct fat_header)) < 0) {
        return -EIO;
    }

    if (header.signature != FAT16_SIGNATURE) {
        return -EMEDIUMTYPE;
    }

    if (strncmp((char *)header.system_id_string, FAT16_SYSTEM_ID, FAT16_SYSTEM_ID_LEN)) {
        return -EMEDIUMTYPE;
    }

    print("Fat16 medium found. Disk #");
    char buffer[10];
    itoa(disk->id, buffer);
    print(buffer);
    print_char('\n');
    diskstream_close(stream);
    return 0;
}

void *fat16_open(struct disk_t *disk, struct path_part *path, enum fopen_mode mode)
{
    print("fopen fat16");
    (void)disk;
    (void)path;
    (void)mode;
    return 0;
}

int fat16_close(struct disk_t *disk)
{
    (void)disk;
    return 0;
}

int fat16_read(struct disk_t *disk)
{
    (void)disk;
    return 0;
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
