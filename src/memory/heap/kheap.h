#ifndef KHEAP_H
#define KHEAP_H
#include "peachos.h"
int kheap_init();
void* kmalloc(size_t size);
void* kzalloc(size_t size);
void kfree(void* ptr);
#endif
