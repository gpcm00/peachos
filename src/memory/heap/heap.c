#include "heap.h"
#include "memory/memory.h"
#include <stdbool.h>

static bool validate_table(void* ptr, void* end, struct heap_table* table)
{
    size_t size = ((size_t)end - (size_t)ptr) / PEACHOS_HEAP_BLOCK_SIZE;
    return size == table->total;
   
}

static bool validate_alignment(void* ptr) {
    return ((unsigned)ptr % PEACHOS_HEAP_BLOCK_SIZE) == 0;
}

static size_t align_size(size_t val)
{
    if (val % PEACHOS_HEAP_BLOCK_SIZE == 0) {
        return val;
    }

    val -= (val % PEACHOS_HEAP_BLOCK_SIZE);
    val += PEACHOS_HEAP_BLOCK_SIZE;
    return val;
}

static HEAP_BLOCK_TABLE_ENTRY get_entry_type(HEAP_BLOCK_TABLE_ENTRY entry)
{
    return entry & 0xF;
}

static int get_start_block(struct heap* heap, uint32_t total_blocks)
{
    struct heap_table* table = heap->table;
    int bs = -1;
    int bc = 0;

    for (int i = 0; i < table->total; i++) {
        if (get_entry_type(table->entries[i]) != HEAP_BLOCK_TABLE_ENTRY_FREE) {
            bc = 0;
            bs = -1;
            continue;
        }

        if (bs == -1) {
            bs = i;
        }
        bc++;

        if (bc == total_blocks) {
            return bs;
        }
    }

    return -ENOMEM;
}

static void* block_to_address(struct heap* heap, int block)
{
    return heap->saddr + (block * PEACHOS_HEAP_BLOCK_SIZE);
}

static int address_to_block(struct heap* heap, void* address)
{
    return (int)(heap->saddr - address) / PEACHOS_HEAP_BLOCK_SIZE;
}

static void mark_block_taken(struct heap* heap, uint32_t start, size_t blocks)
{
    HEAP_BLOCK_TABLE_ENTRY entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_IS_FIRST;
    int last = start + blocks - 1;
    for (int i = start; i <= last; i++) {
        heap->table->entries[i] = entry;
        if (i < last) {
            heap->table->entries[i] |= HEAP_BLOCK_HAS_NEXT;
        }
        entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN;
    }
}

static void mark_block_free(struct heap* heap, uint32_t start)
{
    int total = heap->table->total;
    for (int i = start; i < total; i++) {
        HEAP_BLOCK_TABLE_ENTRY entry = heap->table->entries[i];
        heap->table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;
        if (!(entry & HEAP_BLOCK_HAS_NEXT)) {
            break;
        }
    }
}

static void* malloc_blocks(struct heap* heap, size_t blocks)
{
    int start = get_start_block(heap, blocks);
    if (start < 0) {
        return NULL;
    }

    mark_block_taken(heap, start, blocks);
    return block_to_address(heap, start);
}

int heap_create(struct heap* heap, void* ptr, void* end, struct heap_table* table)
{
    if (!validate_alignment(ptr) || !validate_alignment(end) || !validate_table(ptr, end, table)) {
        return -EINVARG;
    }

    memset(heap, 0, sizeof(struct heap));
    heap->saddr = ptr;
    heap->table = table;

    size_t table_size = sizeof(HEAP_BLOCK_TABLE_ENTRY) * table->total;
    memset(table->entries, 0, table_size);

    return 0;
}

void* heap_malloc(struct heap* heap, size_t size)
{
    size_t blocks = align_size(size) / PEACHOS_HEAP_BLOCK_SIZE;
    return malloc_blocks(heap, blocks);
}
void heap_free(struct heap* heap, void* ptr)
{
    uint32_t start = address_to_block(heap, ptr);
    mark_block_free(heap, start);
}