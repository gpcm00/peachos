#include "peachos.h"
#include "kernel.h"
#include "print/print.h"
#include "idt/idt.h"
#include "io/io.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "memory/fmalloc/fmalloc.h"
#include "fs/pparser.h"

#define KERNEL_CHUNK_FLAGS                                      \
    (PAGING_WRITEABLE | PAGING_PRESENT | PAGING_ACCESS_FROM_ALL)

void panic(const char* msg)
{
    color_print(msg, 0x4F);
    while(1);
}

static struct paging_4gb_chunk kernel_chunk;
void kernel_main()
{
    terminal_init();

    idt_init();

    disk_search_and_init();

    if (kheap_init() < 0) {
        print("Failed to initialize the heap!\n");
        while(1);
    }

    kernel_chunk = paging_new_4gb(KERNEL_CHUNK_FLAGS);
    paging_switch(&kernel_chunk);
    
    enable_paging();
    enable_interrupts();

    // tests --------------------------------------------------
    struct fmalloc_heap heap;
    if (fmalloc_alloc_heap(&heap, 2) < 0) {
        panic("Failed to create FMALLOC heap\n");
    }

    print_raw_bytes(&heap.base, sizeof(void*));
    print_newline();

    size_t sizes[] = {5, 100, 128, 95, 21};
    void* p[sizeof(sizes) / sizeof(sizes[0])];
    int n = sizeof(sizes) / sizeof(sizes[0]);

    for (int i = 0; i < n; i++) {
        p[i] = fmalloc_alloc(&heap, sizes[i]);
        // print_raw_bytes(&p[i], sizeof(void*));
        // print_newline();
    }

    // print_newline();

    for (int i = 0; i < n; i++) {
        uint8_t* ptr = (uint8_t*)p[i];
        for (int j = 0; j < sizes[i]; j++) {
            ptr[j] = j;
        }
    }
    
    // size_t total = (uint32_t)&p[n-1] - (uint32_t)heap.base;
    print_raw_bytes(heap.base, 256);
    
    while(1);
}