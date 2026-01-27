#include "paging.h"
#include "memory/heap/kheap.h"

extern void paging_load_directory(uint32_t* directory);

static uint32_t* current_directory = 0;
struct paging_4gb_chunk paging_new_4gb(uint8_t flags)
{
    uint32_t* directory = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
    uint32_t offset = 0;
    for (int d = 0; d < PAGING_TOTAL_ENTRIES_PER_TABLE; d++) {
        uint32_t* entry = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
        for (int e = 0; e < PAGING_TOTAL_ENTRIES_PER_TABLE; e++) {
            entry[e] = (offset + e * PAGING_PAGE_SIZE) | flags;
        }

        offset += PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE;
        directory[d] = (uint32_t)entry | flags | PAGING_WRITEABLE;
    }

    struct paging_4gb_chunk chunk;
    chunk.directory_entry = directory;
    chunk.flags = flags;

    return chunk;
}

void paging_switch(struct paging_4gb_chunk* directory)
{
    paging_load_directory(directory->directory_entry);
    current_directory = directory->directory_entry;
}

uint32_t *paging_4gb_chunk_get_directory(struct paging_4gb_chunk *chunk)
{
    return chunk->directory_entry;
}

bool paging_is_aligned(void* addr)
{
    return ((uint32_t)addr % PAGING_PAGE_SIZE) == 0;
}

int paging_get_index(void* virtual_addr, uint32_t* dir_out, uint32_t* page_out)
{
    if (!paging_is_aligned(virtual_addr)) {
        return -EINVARG;
    }

    *dir_out = (uint32_t)virtual_addr / (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);
    *page_out = ((uint32_t)virtual_addr % (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE)) / PAGING_PAGE_SIZE;

    return 0;
}

int paging_set(uint32_t* directory, void* virt, uint32_t val)
{
    uint32_t directory_idx = 0;
    uint32_t table_idx = 0;

    if (paging_get_index(virt, &directory_idx, &table_idx) < 0) {
        return -EINVARG;
    }

    uint32_t entry = directory[directory_idx];
    uint32_t* table = (uint32_t*)(entry & ~N_BIT_MASK(12));
    table[table_idx] = val;

    return 0;
}