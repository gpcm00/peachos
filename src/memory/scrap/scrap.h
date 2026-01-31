#ifndef SCRAP_H
#define SCRAP_H

#include "peachos.h"

struct scrap_memory
{
    uint32_t alignment;
    uint32_t next;
    uint8_t data[0];
} __attribute__((packed));

struct scrap_memory* scrap_alloc_memory(uint32_t alignment);
void* scrap_alloc_data(struct scrap_memory* scrap, size_t size);
void scrap_dealloc_data(struct scrap_memory* scrap, size_t size);
void scrap_destroy_memory(void* scrap);

#endif
