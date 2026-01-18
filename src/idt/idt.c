#include "peachos.h"
#include "idt.h"
#include "memory/memory.h"
#include "print/print.h"
#include "io/io.h"

extern void idt_load(struct idtr_desc* idtr_ptr);
extern void no_interrupt();
extern void int21h_isr();

struct idt_desc idt_descriptors[PEACHOS_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

static void idt_zero()
{
    color_print("Divide by zero\n", 0x40);
}

static void idt_set(int int_n, void* address)
{
    struct idt_desc* desc = &idt_descriptors[int_n];
    desc->offset_1 = (uint32_t)address & N_BIT_MASK(16);
    desc->selector = KERNEL_CODE_SELECTOR;
    desc->zero = 0;
    desc->type_attr = 0xEE;
    desc->offset_2 = (uint32_t)address >> 16;
}

void idt_init()
{
    memset(idt_descriptors, 0, sizeof(idt_descriptors));
    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
    idtr_descriptor.base = (uint32_t)idt_descriptors;

    idt_set(0, idt_zero);

    for (int i = 0; i < PEACHOS_TOTAL_INTERRUPTS; i++) {
        idt_set(i, no_interrupt);
    }

    idt_set(0x21, int21h_isr);


    idt_load(&idtr_descriptor);
}

void int21h_handler()
{
    print("Keyboard pressed\n");
    outb(0x20, 0x20);
}

void no_interrupt_handler()
{
    outb(0x20, 0x20);
}