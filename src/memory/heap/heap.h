#ifndef HEAP_H
#define HEAP_H
#include "peachos.h"
#include <stdint.h>
#include <stddef.h>

#define HEAP_BLOCK_TABLE_ENTRY_FREE     0x00
#define HEAP_BLOCK_TABLE_ENTRY_TAKEN    BIT(0)

#define HEAP_BLOCK_HAS_NEXT             BIT(7)
#define HEAP_BLOCK_IS_FIRST             BIT(6)


typedef unsigned char HEAP_BLOCK_TABLE_ENTRY;

struct heap_table
{
    HEAP_BLOCK_TABLE_ENTRY* entries;
    size_t total;
};


struct heap
{
    struct heap_table* table;
    void* saddr;
};

int heap_create(struct heap* heap, void* ptr, void* end, struct heap_table* table);
void* heap_malloc(struct heap* heap, size_t size);
void heap_free(struct heap* heap, void* ptr);

#endif
