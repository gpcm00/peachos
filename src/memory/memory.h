#ifndef MEMORY_H
#define MEMORY_H

#include "peachos.h"

void* memset(void* ptr, int c, size_t len);
void* memcpy(void* dst, const void* src, size_t size);

#endif