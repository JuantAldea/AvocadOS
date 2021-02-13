#ifndef __VFS_H__
#define __VFS_H__

#include <stddef.h>
#include "fs_types.h"

struct file_descriptor;
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
struct filesystem_operations *vfs_discover_fs(struct disk *disk);

#endif
