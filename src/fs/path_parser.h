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


#endif
