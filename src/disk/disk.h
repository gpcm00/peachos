#ifndef DISK_H
#define DISK_H

#include "peachos.h"
#include "fs/file.h"


typedef enum PEACHOS_DISK_TYPE {
    PEACHOS_DISK_TYPE_REAL,
} PEACHOS_DISK_TYPE;

struct disk {
    PEACHOS_DISK_TYPE type;
    size_t sector_size;

    struct filesystem* filesystem;

    int disk_id;

    void* private_data;
};

void disk_search_and_init();
struct disk* disk_get(int index);
int disk_read_block(struct disk* disk, int lba, size_t total, void* buf);

#endif