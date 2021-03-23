#ifndef __FILE_TABLE_H__
#define __FILE_TABLE_H__

#include "../config.h"
#include "../fs/file.h"

#include <stddef.h>

struct file_table_t
{
    struct FILE *table[MAX_OPEN_FILES];
    int table_len;
};

extern struct file_table_t file_table;

int file_table_open_file(struct FILE *descriptor);
int file_table_close_file(struct FILE *descriptor);
struct FILE *file_table_get_descriptor(int fd);

int file_table_init();

#endif
