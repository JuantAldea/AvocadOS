#ifndef _DISK_STREAM_H
#define _DISK_STREAM_H
#include <stddef.h>

struct disk;

struct disk_stream {
    size_t ppos;
    struct disk *disk;
};

struct disk_stream *diskstream_new(int disk_id);
void diskstream_close(struct disk_stream *stream);
int diskstream_read(struct disk_stream *stream, void *buffer, int count);
int diskstream_seek(struct disk_stream *stream, size_t ppos);

#endif
