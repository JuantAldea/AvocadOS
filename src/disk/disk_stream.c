#include "disk_stream.h"
#include "../memory/memory.h"
#include "../memory/kheap.h"

int diskstream_seek(struct disk_stream *stream, size_t ppos)
{
    stream->ppos = ppos;
    return ppos;
}

struct disk_stream *diskstream_new(int disk_id)
{
    struct disk *disk = disk_get(disk_id);

    if (!disk) {
        return NULL;
    }

    struct disk_stream *stream = kzalloc(sizeof(struct disk_stream));
    stream->disk = disk;
    return stream;
}

int diskstream_read(struct disk_stream *stream, void *buffer, int count)
{
    int offset = stream->ppos % DISK_SECTOR_SIZE;
    int sector = stream->ppos / DISK_SECTOR_SIZE;
    char sector_buffer[DISK_SECTOR_SIZE];
    int read_so_far = 0;

    while (read_so_far < count) {
        const int ret = disk_read_block(stream->disk, sector, 1, sector_buffer);

        if(ret) {
            return ret;
        }

        size_t to_copy = count < (DISK_SECTOR_SIZE - offset) ? count : (DISK_SECTOR_SIZE - offset);
        memcpy(buffer + read_so_far, sector_buffer + offset, to_copy);
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
