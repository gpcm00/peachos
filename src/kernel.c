#include "peachos.h"
#include "kernel.h"
#include "print/print.h"
#include "idt/idt.h"
#include "io/io.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"

#define KERNEL_CHUNK_FLAGS                                      \
    (PAGING_WRITEABLE | PAGING_PRESENT | PAGING_ACCESS_FROM_ALL)

static struct paging_4gb_chunk kernel_chunk;
void kernel_main()
{
    terminal_init();
    print("Hello world!\n");

    idt_init();

    if (kheap_init() < 0) {
        print("Failed to initialize the heap!\n");
        while(1);
    }

    kernel_chunk = paging_new_4gb(KERNEL_CHUNK_FLAGS);
    paging_switch(&kernel_chunk);
    
    const uint32_t addr = 0x1000;
    char* ptr = kzalloc(4096);
    if (paging_set(kernel_chunk.directory_entry, (void*)addr, (uint32_t)ptr | KERNEL_CHUNK_FLAGS) < 0) {
        print("panic\n");
        while (1);
    }

    const char str[] = "Testing paging\n";
    paging_switch(&kernel_chunk);
    enable_paging();
    enable_interrupts();

    char* ptr2 = (char*)addr;
    memcpy(ptr2, str, sizeof(str));
    if (ptr2 == 0) {
        print("nothing\n");
        while (1);
    }

    print(ptr);
    print(ptr2);
    
    while(1);
}