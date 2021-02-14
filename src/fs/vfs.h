#ifndef __VFS_H__
#define __VFS_H__

#include <stddef.h>

struct file_descriptor;
struct disk;

enum seek_operation
{
    SEEK_SET = 0,
    SEEK_CUR,
    SEEK_END,
};

enum fopen_mode
{
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

struct filesystem_operations
{
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

extern struct filesystem_operations *registered_filesystems[10];

int vfs_open(struct file_descriptor *descriptor, enum fopen_mode mode);
int vfs_close(struct file_descriptor *descriptor);
int vfs_seek(struct file_descriptor *descriptor);
int vfs_link(struct file_descriptor *descriptor);
int vfs_unlink(struct file_descriptor *descriptor);
int vfs_write(struct file_descriptor *descriptor);
int vfs_read(struct file_descriptor *descriptor);
int vfs_stat(struct file_descriptor *descriptor);

int vfs_init();
int vfs_register_fs(struct filesystem_operations *operations);

struct filesystem_operations *vfs_probe_filesystem(struct disk *disk);

#endif
