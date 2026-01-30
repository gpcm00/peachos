#ifndef DISK_H
#define DISK_H

#include "peachos.h"


typedef enum PEACHOS_DISK_TYPE {
    PEACHOS_DISK_TYPE_REAL,
} PEACHOS_DISK_TYPE;

struct disk {
    PEACHOS_DISK_TYPE type;
    size_t sector_size;
};

void disk_search_and_init();
struct disk* get_disk(int index);
int disk_read_block(struct disk* disk, int lba, size_t total, void* buf);

#endif