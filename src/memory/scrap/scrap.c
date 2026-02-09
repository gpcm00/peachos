#include "scrap.h"
#include "memory/heap/kheap.h"

static bool validate_alignment(uint32_t alignment)
{
    return (alignment != 0) && ((alignment & (alignment-1)) == 0);
}

static uint32_t align(size_t addr, uint32_t alignment)
{
    return (addr + (alignment-1)) & ~(alignment-1);
}


struct scrap_memory* scrap_alloc_memory(uint32_t alignment)
{
    if (!validate_alignment(alignment)) {
        return NULL;
    }

    struct scrap_memory* scrap = kzalloc(PEACHOS_HEAP_BLOCK_SIZE);
    if (scrap != NULL) {
        scrap->alignment = alignment;
        scrap->next = align(sizeof(struct scrap_memory), alignment);
    }
    
    return scrap;
}

void* scrap_alloc_data(struct scrap_memory* scrap, size_t size)
{
    size = align(size, scrap->alignment);
    if ((size == 0) || ((scrap->next + size) > PEACHOS_HEAP_BLOCK_SIZE)) {
        return NULL;
    }

    uint32_t base = (uint32_t)scrap + scrap->next;
    scrap->next += size;
    
    return (void*)base;
}

void scrap_dealloc_data(struct scrap_memory* scrap, size_t size)
{
    scrap->next -= align(size, scrap->alignment);
}

void scrap_destroy_memory(void* scrap)
{
    uint32_t base = (uint32_t)scrap;
    base -= base % PEACHOS_HEAP_BLOCK_SIZE;
    kfree((void*)base);
}