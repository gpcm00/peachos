#ifndef PPARSER_H
#define PPARSER_H
#include "peachos.h"
#include "memory/scrap/scrap.h"

struct path_root
{
    int drive_no;
    struct path_part* first;
};

struct path_part
{
    const char* part;
    struct path_part* next;
};

#endif
