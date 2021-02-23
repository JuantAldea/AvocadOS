#include "disk_stream.h"
#include "../string/string.h"
#include "../memory/kheap.h"
#include "disk.h"

int diskstream_seek(struct disk_stream *stream, size_t ppos)
{
    stream->ppos = ppos;
    return ppos;
}

struct disk_stream *diskstream_new(int disk_id)
{
    struct disk_t *disk;
    int res = disk_get(disk_id, &disk);

    if (res) {
        return NULL;
    }

    struct disk_stream *stream = kzalloc(sizeof(struct disk_stream));
    stream->disk = disk;
    return stream;
}

int diskstream_read(struct disk_stream *stream, void *buffer, int len)
{
    int offset = stream->ppos % DISK_SECTOR_SIZE;
    int sector = stream->ppos / DISK_SECTOR_SIZE;
    char sector_buffer[DISK_SECTOR_SIZE];
    int read_so_far = 0;

    while (read_so_far < len) {
        const int ret = disk_read_block(stream->disk, sector, 1, sector_buffer);

        if (ret) {
            return ret;
        }

        const int remaining = len - read_so_far;
        const size_t to_copy = remaining < (DISK_SECTOR_SIZE - offset) ? remaining : (DISK_SECTOR_SIZE - offset);
        memcpy(buffer + read_so_far, sector_buffer + offset, to_copy); // NOLINT
        read_so_far += to_copy;
        // there could be offset only for the first chunk read
        offset = 0;
        /* either the read requires less or equal of what's remaining of the sector, and it's done
        or it requires more, so it has to reach for the next sector */
        sector++;
    }

    stream->ppos += read_so_far;
    return read_so_far;
}

void diskstream_close(struct disk_stream *stream)
{
    kfree(stream);
}
