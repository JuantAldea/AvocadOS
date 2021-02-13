#ifndef __PATH_PARSER_H
#define __PATH_PARSER_H
struct path_root
{
    int drive_number;
    struct path_part *first;
};

struct path_part
{
    char *part;
    struct path_part *next;
};

void pathparse_path_free(struct path_root *path);
struct path_root *pathparser_path_parse(const char * const path);

#endif
