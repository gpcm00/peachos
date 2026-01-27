#ifndef PAGING_H
#define PAGING_H

#include "peachos.h"

#define PAGING_PRESENT          BIT(0)
#define PAGING_WRITEABLE        BIT(1)
#define PAGING_ACCESS_FROM_ALL  BIT(2)
#define PAGING_WRITE_THROUGH    BIT(3)
#define PAGING_CACHE_DISABLE    BIT(4)

#define PAGING_TOTAL_ENTRIES_PER_TABLE 1024
#define PAGING_PAGE_SIZE 4096

struct paging_4gb_chunk
{
    uint32_t* directory_entry;
    uint8_t flags;
};

void enable_paging();
struct paging_4gb_chunk paging_new_4gb(uint8_t flags);
void paging_switch(struct paging_4gb_chunk* directory);
uint32_t *paging_4gb_chunk_get_directory(struct paging_4gb_chunk *chunk);
bool paging_is_aligned(void* addr);
int paging_set(uint32_t* directory, void* virt, uint32_t val);

#endif
