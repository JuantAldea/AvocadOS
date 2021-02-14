#ifndef __FILE_H__
#define __FILE_H__

struct disk;
struct path_part;

struct file_descriptor {
    struct disk *disk;
    struct path_root *path;
    void *private_data;
};

#endif
