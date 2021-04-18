#include "file_table.h"
#include "../memory/kheap.h"
#include "../fs/file.h"
#include "../status.h"
#include "../string/string.h"

struct file_table_t file_table;
int file_table_find_free_slot();

int file_table_init()
{
    file_table.table_len = MAX_OPEN_FILES;
    memset(file_table.table, 0, sizeof(file_table.table)); //NOLINT

    return 0;
}

int file_table_find_free_slot()
{
    for (int i = 0; i < file_table.table_len; ++i) {
        if (!file_table.table[i]) {
            return i;
        }
    }

    return -EMFILE;
}

int file_table_open_file(struct FILE *descriptor)
{
    int index = file_table_find_free_slot();

    if (index < 0) {
        return index;
    }

    descriptor->fileno = index;
    file_table.table[index] = descriptor;

    return index;
}

int file_table_close_file(struct FILE *descriptor)
{
    if (0 > descriptor->fileno || descriptor->fileno >= file_table.table_len) {
        return -EBADF;
    }

    file_table.table[descriptor->fileno] = NULL;
    descriptor->fileno = -1;

    return 0;
}

struct FILE *file_table_get_descriptor(int fd)
{

    if (fd < 0 || fd >= file_table.table_len) {
        return NULL;
    }

    return file_table.table[fd];

}
