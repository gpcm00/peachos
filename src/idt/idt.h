#ifndef IDT_H
#define IDT_H
#include <stdint.h>
#include <stddef.h>

struct idt_desc {
    uint16_t offset_1;          // bits 0 - 15
    uint16_t selector;          // selector in GDT
    uint8_t zero;               // unused
    uint8_t type_attr;          // desc type and attributes
    uint16_t offset_2;          // bits 16-31
} __attribute__((packed));

struct idtr_desc
{
    uint16_t limit;             // size of descriptor -1
    uint32_t base;              // base address of descriptor
} __attribute__((packed));

void idt_init();

#endif