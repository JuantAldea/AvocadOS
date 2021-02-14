#include "fat16.h"
#include "../disk/disk_stream.h"
#include "../status.h"
#include "../status.h"
#include "../string/string.h"
#include "../termio/termio.h"
#include "vfs.h"
#include "../disk/disk.h"

struct filesystem_operations fat16_operations = {
    .resolve = fat16_resolve,
    .open = fat16_open,
    .close = fat16_close,
    .read = fat16_read,
    .write = fat16_write,
    .seek = fat16_seek,
    .stat = fat16_stat,
    .link = fat16_link,
    .unlink = fat16_unlink,
};

int fat16_resolve(struct disk *disk)
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

int fat16_open(struct disk *disk, enum fopen_mode mode)
{
    (void)disk;
    (void)mode;
    return 0;
}

int fat16_close(struct disk *disk)
{
    (void)disk;
    return 0;
}

int fat16_read(struct disk *disk)
{
    (void)disk;
    return 0;
}

int fat16_write(struct disk *disk)
{
    (void)disk;
    return 0;
}

int fat16_seek(struct disk *disk)
{
    (void)disk;
    return 0;
}

int fat16_stat(struct disk *disk)
{
    (void)disk;
    return 0;
}

int fat16_link(struct disk *disk)
{
    (void)disk;
    return 0;
}

int fat16_unlink(struct disk *disk)
{
    (void)disk;
    return 0;
}
