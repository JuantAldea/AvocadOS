#include "vfs.h"
#include "../config.h"
#include "../status.h"
#include "../disk/disk.h"
#include "../kernel/kernel.h"
#include "fat16.h"

struct filesystem_operations *registered_filesystems[10];

int vfs_resolve(struct file_descriptor *descriptor)
{
    return descriptor->disk->fs_operations->resolve(descriptor->disk);
}

int vfs_open(struct file_descriptor *descriptor, enum fopen_mode mode)
{
    return descriptor->disk->fs_operations->open(descriptor->disk, mode);
}

int vfs_close(struct file_descriptor *descriptor)
{
    return descriptor->disk->fs_operations->close(descriptor->disk);
}

int vfs_read(struct file_descriptor *descriptor)
{
    return descriptor->disk->fs_operations->read(descriptor->disk);
}

int vfs_write(struct file_descriptor *descriptor)
{
    return descriptor->disk->fs_operations->write(descriptor->disk);
}

int vfs_seek(struct file_descriptor *descriptor)
{
    return descriptor->disk->fs_operations->seek(descriptor->disk);
}

int vfs_stat(struct file_descriptor *descriptor)
{
    return descriptor->disk->fs_operations->stat(descriptor->disk);
}

int vfs_link(struct file_descriptor *descriptor)
{
    return descriptor->disk->fs_operations->link(descriptor->disk);
}

int vfs_unlink(struct file_descriptor *descriptor)
{
    return descriptor->disk->fs_operations->unlink(descriptor->disk);
}

int vfs_init()
{
    vfs_register_fs(&fat16_operations);
    return 0;
}

int vfs_register_fs(struct filesystem_operations *operations)
{
    for (size_t i = 0; i < MAX_FILESYSTEMS; ++i) {
        if (!registered_filesystems[i]) {
            registered_filesystems[i] = operations;
            return 0;
        }
    }

    return -EIO;
}

struct filesystem_operations *vfs_discover_fs(struct disk *disk)
{
    struct filesystem_operations *fs_op = registered_filesystems[0];
    for (; fs_op; ++fs_op){
        if (!fs_op->resolve(disk)) {
            return fs_op;
        }
    }

    return NULL;
}
