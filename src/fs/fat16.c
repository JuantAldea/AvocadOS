#include "fat16.h"
#include "../disk/disk_stream.h"
#include "../status.h"
#include "../status.h"
#include "../string/string.h"
#include "../termio/termio.h"

struct filesystem_operations fat16_operations =
{
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
    struct disk_stream *stream = diskstream_open(disk->id);

    struct fat_header header = {0};

    diskstream_read(stream, &header, sizeof(struct fat_header));

    if (header.signature != 0x29){
        return -EMEDIUMTYPE;
    }

    if (strncmp((char*)header.system_id_string, "FAT16   ", 8)) {
        return -EMEDIUMTYPE;
    }
    print("Fat16 medium found. Disk #");
    print_char(digit_to_char(disk->id));
    print_char('\n');
    return 0;
}

int fat16_open(struct disk *disk, enum fopen_mode mode)
{
    return 0;
}

int fat16_close(struct disk *disk)
{
    return 0;
}

int fat16_read(struct disk *disk)
{
    return 0;
}

int fat16_write(struct disk *disk)
{
    return 0;
}

int fat16_seek(struct disk *disk)
{
    return 0;
}

int fat16_stat(struct disk *disk)
{
    return 0;
}

int fat16_link(struct disk *disk)
{
    return 0;
}

int fat16_unlink(struct disk *disk)
{
    return 0;
}
