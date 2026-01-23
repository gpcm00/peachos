#include "peachos.h"
#include "kernel.h"
#include "print/print.h"
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"

void kernel_main()
{
    terminal_init();
    print("Hello world!\n");

    idt_init();
    enable_interrupts();

    if (kheap_init() < 0) {
        print("Failed to initialize the heap!\n");
        while(1);
    }

    void* addr1 = kmalloc(0x100);
    void* addr2 = kmalloc(0x1001);
    void* addr3 = kmalloc(0x10);
    kfree(addr1);
    void* addr4 = kmalloc(0x10);

    if (addr1 || addr2 || addr3 || addr4) {
        print("End of test!\n");
    }

    while(1);
}