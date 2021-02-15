#include "file.h"
#include "file_table.h"
#include "path_parser.h"
#include "../disk/disk.h"
#include "../status.h"
#include "fat16.h"
#include "../termio/termio.h"

struct filesystem_operations_t *registered_filesystems[10];

int fs_init()
{
    return fs_register_fs(&fat16_operations);
}

int fs_register_fs(struct filesystem_operations_t *operations)
{
    for (size_t i = 0; i < MAX_FILESYSTEMS; ++i) {
        if (!registered_filesystems[i]) {
            registered_filesystems[i] = operations;
            return 0;
        }
    }

    return -EIO;
}

struct filesystem_operations_t *fs_probe_fs(struct disk_t *disk)
{
    struct filesystem_operations_t *fs_op = registered_filesystems[0];
    for (; fs_op; ++fs_op) {
        if (!fs_op->probe(disk)) {
            return fs_op;
        }
    }

    return NULL;
}

int fopen(const char *const filename, enum fopen_mode mode)
{
    struct path_root *path = pathparser_path_parse(filename);
    struct file_descriptor_t *descriptor;
    int res = file_table_open_file(&descriptor);

    if (res) {
        pathparse_path_free(path);
        return res;
    }

    descriptor->disk = disk_get(path->drive_number);

    if (!descriptor->disk) {
        return -EINVAL;
    }

    descriptor->path = path;
    descriptor->private_data = descriptor->disk->fs_operations->open(descriptor->disk, path->first, mode);

    return descriptor->fileno;
}