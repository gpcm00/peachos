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
#include "memory/scrap/scrap.h"
#include "fs/pparser.h"

#define KERNEL_CHUNK_FLAGS                                      \
    (PAGING_WRITEABLE | PAGING_PRESENT | PAGING_ACCESS_FROM_ALL)


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
    struct disk_streamer stream = diskstreamer_new(0);
    diskstreamer_seek(&stream, 510);

    char buffer[64];
    memset(buffer, 0, sizeof(buffer));

    diskstreamer_read(&stream, buffer, sizeof(buffer));

    print_raw_bytes(buffer, sizeof(buffer));

    while(1);
}