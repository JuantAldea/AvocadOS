#ifndef __FS_TYPES_H__
#define __FS_TYPES_H__
/*
enum seek_operation {
    SEEK_SET = 0,
    SEEK_CUR,
    SEEK_END,
};

enum fopen_mode {
    OPEN_MODE_READ = 0,
    OPEN_MODE_WRITE,
    OPEN_MODE_RW,
};

typedef int (*FS_RESOLVE)(struct disk *disk);
typedef int (*FS_OPEN)(struct disk *disk, enum fopen_mode mode);
typedef int (*FS_CLOSE)(struct disk *disk);
typedef int (*FS_READ)(struct disk *disk);
typedef int (*FS_WRITE)(struct disk *disk);
typedef int (*FS_SEEK)(struct disk *disk);
typedef int (*FS_STAT)(struct disk *disk);
typedef int (*FS_LINK)(struct disk *disk);
typedef int (*FS_UNLINK)(struct disk *disk);

struct filesystem_operations {
    FS_RESOLVE resolve;
    FS_OPEN open;
    FS_CLOSE close;
    FS_READ read;
    FS_WRITE write;
    FS_SEEK seek;
    FS_STAT stat;
    FS_LINK link;
    FS_UNLINK unlink;
};

struct file_descriptor {
    struct disk *disk;
    struct path_root *path;
    void *private_data;
};
*/
#endif
