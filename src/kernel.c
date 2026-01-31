#include "peachos.h"
#include "kernel.h"
#include "print/print.h"
#include "idt/idt.h"
#include "io/io.h"
#include "disk/disk.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "memory/scrap/scrap.h"

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

    struct disk* disk = get_disk(0);

    struct scrap_memory* scrap = scrap_alloc_memory(512);

    char* buff = scrap_alloc_data(scrap, 512);
    if (disk_read_block(disk, 0, 1, buff) < 0){
        print ("PANIC\n");
    }

    print_raw_bytes(&scrap, sizeof(struct scrap_memory*));
    print("\n");

    print_raw_bytes(buff, sizeof(buff));
    print("\n");
    print_raw_bytes(scrap, sizeof(struct scrap_memory));
    print("\n");

    buff = scrap_alloc_data(scrap, 1);
    print_raw_bytes(scrap, sizeof(struct scrap_memory));
    print("\n");

    buff = scrap_alloc_data(scrap, 600);
    print_raw_bytes(scrap, sizeof(struct scrap_memory));
    print("\n");

    scrap_dealloc_data(scrap, 1);
    print_raw_bytes(scrap, sizeof(struct scrap_memory));
    print("\n");

    print_raw_bytes(&buff, sizeof(struct scrap_memory*));
    print("\n");

    int calc = (uint32_t)buff % PEACHOS_HEAP_BLOCK_SIZE;
    calc = (uint32_t)buff - calc;
    print_raw_bytes(&calc, sizeof(int));
    print("\n");

    scrap_destroy_memory(scrap);
    // kfree(scrap);
    char* tst = kmalloc(5000);
    print_raw_bytes(&tst, sizeof(char*));
    print("\n");

    kfree(tst);

    tst = kmalloc(1);
    print_raw_bytes(&tst, sizeof(char*));
    print("\n");

    while(1);
}