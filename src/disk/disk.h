#ifndef DISK_H
#define DISK_H

#include "peachos.h"

int disk_read_sector(int lba, size_t total, void* buf);

#endif
