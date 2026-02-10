#ifndef BINS_H
#define BINS_H

#include "peachos.h"

struct bins_memory 
{
    uint32_t bin_size;
    void* next;
    uint8_t data[0];
} __attribute__((packed));

struct bins_memory* bins_alloc_memory(size_t size, uint32_t bin_size);
void* bins_alloc_data(struct bins_memory* bin, size_t size);
int bins_dealloc_data(struct bins_memory* bin, void* ptr);
void bins_destroy_memory(void* bin);

#endif
