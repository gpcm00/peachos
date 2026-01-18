#include "peachos.h"
#include "kernel.h"
#include "print/print.h"
#include "idt/idt.h"
#include "io/io.h"

void kernel_main()
{
    terminal_init();
    print("Hello world!\n");

    idt_init();

    // uint8_t b = insb(0x60);
    // uint16_t w = insw(0x60) + b;

    // w ^= w;

    // outb(0x60, 0xFF);
    // outw(0x60, 0xFF);
    while(1);
}