#include "path_parser.h"

#include "../config.h"
#include "../string/string.h"
#include "../memory/memory.h"
#include "../memory/kheap.h"

#include "../termio/termio.h"

static int pathparser_path_get_drive(const char * const path);
static int pathparser_path_is_valid(const char * path);

static int pathparser_path_is_valid(const char * path)
{
    const int len = strnlen(path, MAX_PATH_LEN);
    return len >= 3 && is_digit(path[0]) && !memcmp(":/", &path[1], 2);
}

static int pathparser_path_get_drive(const char * const path)
{
    return numeric_char_to_digit(path[0]);
}

struct path_root *pathparser_path_parse(const char * const path)
{
    if (!pathparser_path_is_valid(path)) {
        return NULL;
    }

    const char * const path_end = path + (strnlen(path, MAX_PATH_LEN));

    struct path_root *p = kzalloc(sizeof(struct path_root));

    if (!p) {
        return NULL;
    }

    const char *path_ptr = path;
    p->drive_number = pathparser_path_get_drive(path_ptr);

    if (p->drive_number < 0) {
        goto error_out;
    }

    path_ptr += 3;
    struct path_part **current_segment = &p->first;
    while(path_ptr <= path_end) {
        const char *next_slash = (char*) memchr(path_ptr, '/', path_end - path_ptr);
        next_slash = next_slash != NULL ? next_slash : path_end;
        size_t part_len = next_slash - path_ptr;

        if (!part_len) {
            break;
        }

        *current_segment = kzalloc(sizeof(struct path_part));

        (*current_segment)->part = kzalloc((part_len + 1) * sizeof(char));
        memcpy((*current_segment)->part, path_ptr, part_len);
        path_ptr += part_len + 1;
        current_segment = &((*current_segment)->next);
    }

    return p;

error_out:;
    pathparse_path_free(p);
    return NULL;
}

void pathparse_path_free(struct path_root *path)
{
    struct path_part *ptr = path->first;

    while(ptr) {
        struct path_part *next = ptr->next;
        if (ptr->part) kfree(ptr->part);
        kfree(ptr);
        ptr = next;
    }

    kfree(path);
}
