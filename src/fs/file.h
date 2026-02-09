#ifndef FILE_H
#define FILE_H

#include "peachos.h"
#include "disk/disk.h"
#include "pparser.h"

typedef unsigned int FILE_SEEK;
enum {
    SEEK_SET,
    SEEK_CUR,
    SEEK_END,
};

typedef unsigned int FILE_MODE;
enum {
    FILE_MODE_READ,
    FILE_MODE_WRITE,
    FILE_MODE_APPEND,
    FILE_MODE_INVALID,
};

typedef void* (*FS_OPEN_FUNCTION)(struct disk*, struct path_part*, FILE_MODE);
typedef int (*FS_RESOLVE_FUNCTION)(struct disk*);

struct filesystem
{
    FS_RESOLVE_FUNCTION resolve;
    FS_OPEN_FUNCTION open;

    char name[20];
};

struct file_descriptor
{
    int index;
    struct filesystem* filesystem;

    void* private_data;

    struct disk* disk;
};

int fopen(const char* filename, const char* mode);

void fs_init();
void fs_insert_filesystem(struct filesystem* filesystem);
struct filesystem* fs_resolve(struct disk* disk);
#endif
