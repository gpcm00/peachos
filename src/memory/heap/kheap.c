#include "kheap.h"
#include "heap.h"
#include "memory/memory.h"

struct heap kernel_heap;
struct heap_table kernel_heap_table;

int kheap_init()
{
    kernel_heap_table.total = PEACHOS_HEAP_SIZE_BYTES / PEACHOS_HEAP_BLOCK_SIZE;
    kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*) (PEACHOS_HEAP_TABLE_ADDRESS);

    void *start = (void*)(PEACHOS_HEAP_ADDRESS);
    void *end = (void*)(PEACHOS_HEAP_ADDRESS + PEACHOS_HEAP_SIZE_BYTES);

    return heap_create(&kernel_heap, start, end, &kernel_heap_table);
}

void* kmalloc(size_t size)
{
    return heap_malloc(&kernel_heap, size);
}

void* kzalloc(size_t size)
{
    void* ret = heap_malloc(&kernel_heap, size);
    if (ret == NULL) {
        return NULL;
    }

    memset(ret, 0, size);

    return ret;
}

void kfree(void* ptr)
{
    heap_free(&kernel_heap, ptr);
}