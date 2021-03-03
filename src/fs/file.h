#ifndef __FILE_H__
#define __FILE_H__
#include <stddef.h>
#include <stdint.h>
struct disk_t;
struct path_part;
extern struct filesystem_operations_t *registered_filesystems[10];

struct filesystem_operations_t *fs_probe_fs(struct disk_t *disk);
int fs_init();
int fs_register_fs(struct filesystem_operations_t *operations);

struct file_descriptor_t {
    struct disk_t *disk;
    struct path_root *path;
    void *private_data;
    int fileno;
};

enum seek_operation {
    SEEK_SET = 0,
    SEEK_CUR,
    SEEK_END,
};

enum fopen_mode {
    OPEN_MODE_INVALID = 0,
    OPEN_MODE_READ,
    OPEN_MODE_WRITE,
    OPEN_MODE_RW,
    OPEN_MODE_APPEND,
};

struct stat {
    int st_dev;
    uint32_t st_size;
};

typedef int (*FS_PROBE)(struct disk_t *disk);
typedef void *(*FS_OPEN)(struct disk_t *disk, struct path_part *path, enum fopen_mode mode);
typedef int (*FS_CLOSE)(void *priv);
typedef size_t (*FS_READ)(void *priv, uint32_t size, uint32_t nmemb, char *out);
typedef size_t (*FS_WRITE)(struct file_descriptor_t file);
typedef int (*FS_SEEK)(void *priv, int32_t offset, enum seek_operation whence);
typedef int (*FS_STAT)(void *priv, struct stat *buf);
typedef int (*FS_UNLINK)(struct path_root *path);

struct filesystem_operations_t {
    FS_PROBE probe;
    FS_OPEN open;
    FS_CLOSE close;
    FS_READ read;
    FS_WRITE write;
    FS_SEEK seek;
    FS_STAT stat;
    FS_UNLINK unlink;
};

int fopen(const char *const filename, const char *str_mode);
int fclose(int fileno);
size_t fread(int fd, void *ptr, uint32_t size, uint32_t nmemb);
int fseek(int fileno, int32_t offset, enum seek_operation whence);
int fstat(int fileno, struct stat *buf);

#endif
