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
#include "memory/bins/bins.h"
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
    struct bins_memory* heap = bins_alloc_memory(PEACHOS_HEAP_BLOCK_SIZE, 16);
    print_raw_bytes(&heap, sizeof(void*));
    print_newline();
    print_newline();

    // print_raw_bytes(heap, 32);
    // print_newline();

    size_t sizes[] = {50, 9, 11, 14, 20};
    const int len = sizeof(sizes) / sizeof(sizes[0]);
    void* p[len];
    for (int i = 0; i < len; i++) {
        p[i] = bins_alloc_data(heap, sizes[i]);
        if (!p[i]) panic("Failed");
        // print_raw_bytes(&p[i], sizeof(void*));
        // print_newline();

        // if (p[i] != NULL) {
        //     print_raw_bytes(p[i] - 16, 16);
        // }
    }
    if (bins_dealloc_data(heap, p[2]) < 0) panic("Failed to dealloc");
    if (bins_dealloc_data(heap, p[1]) < 0) panic("Failed to dealloc");
    if (bins_dealloc_data(heap, p[3]) < 0) panic("Failed to dealloc");
    
    print_raw_bytes(heap, 256);
    print_newline();

    // print_newline();

    // for ()
    
    while(1);
}