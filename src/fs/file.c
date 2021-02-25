#include "file.h"
#include "file_table.h"
#include "path_parser.h"
#include "../disk/disk.h"
#include "../status.h"
#include "fat16.h"
#include "../termio/termio.h"
#include "../memory/kheap.h"
#include "../string/string.h"

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

enum fopen_mode file_mode_from_string(const char *str)
{
    if (!strncmp(str, "w", 1)) {
        return OPEN_MODE_WRITE;
    } else if (!strncmp(str, "r", 1)) {
        return OPEN_MODE_READ;
    } else if (!strncmp(str, "a", 1)) {
        return OPEN_MODE_APPEND;
    }
    return OPEN_MODE_INVALID;
}

int fopen(const char *const filename, const char *str_mode)
{
    enum fopen_mode mode = file_mode_from_string(str_mode);

    if (mode == OPEN_MODE_INVALID) {
        return -EINVAL;
    }

    int res = 0;
    struct path_root *path = pathparser_path_parse(filename);

    if (!path) {
        return -ENOMEM;
    }

    if (!path->first) {
        return -EINVAL;
    }

    struct file_descriptor_t *descriptor = kzalloc(sizeof(struct file_descriptor_t));
    if (!descriptor) {
        res = -ENOMEM;
        goto out;
    }

    res = disk_get(path->drive_number, &descriptor->disk);

    if (res) {
        res = -EIO;
        goto out;
    }

    if (!descriptor->disk->fs_operations) {
        res = -EIO;
        goto out;
    }

    descriptor->path = path;

    descriptor->private_data = descriptor->disk->fs_operations->open(descriptor->disk, path->first, mode);

    if (!descriptor->private_data) {
        res = -EIO;
        goto out;
    }

    res = file_table_open_file(descriptor);

out:
    if (res) {
        if (path) {
            pathparse_path_free(path);
        }

        if (descriptor && descriptor->disk->fs_operations && descriptor->private_data) {
            descriptor->disk->fs_operations->close(descriptor->private_data);
        }

        if (descriptor) {
            kfree(descriptor);
        }
    }

    return res;
}

int fclose(int fd)
{
    struct file_descriptor_t *descriptor = file_table.table[fd];

    if (!descriptor) {
        return -EBADF;
    }

    int res = file_table_close_file(descriptor);

    if (res) {
        goto out;
    }

    res = descriptor->disk->fs_operations->close(descriptor->private_data);

    if (res) {
        goto out;
    }

    pathparse_path_free(descriptor->path);

    kfree(descriptor);
out:
    return res;
}

int fread(int fd, void *ptr, uint32_t size, uint32_t nmemb)
{
    struct file_descriptor_t *descriptor = file_table.table[fd];

    if (!descriptor) {
        return -EBADF;
    }

    return descriptor->disk->fs_operations->read(descriptor->private_data, size, nmemb, ptr);
}

int fseek(int fd, int32_t offset, enum seek_operation whence)
{
    struct file_descriptor_t *descriptor = file_table.table[fd];

    if (!descriptor) {
        return -EBADF;
    }

    return descriptor->disk->fs_operations->seek(descriptor->private_data, offset, whence);
}

int fstat(int fd, struct stat *buf)
{
    struct file_descriptor_t *descriptor = file_table.table[fd];

    if (!descriptor) {
        return -EBADF;
    }

    return descriptor->disk->fs_operations->stat(descriptor->private_data, buf);
}