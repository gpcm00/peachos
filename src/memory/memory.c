#include "memory.h"

void* memset(void* ptr, int c, size_t len)
{
    char* c_ptr = (char*)ptr;
    for (size_t i = 0; i < len; i++) {
        *(c_ptr++) = (char)c;
    }

    return ptr;
}

void* memcpy(void* dst, const void* src, size_t size)
{
    char* c_dst = (char*)dst;
    const char* c_src = (const char*)src;
    for (size_t i = 0; i < size; i++) {
        c_dst[i] = c_src[i];
    }

    return dst;
}