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

int memcmp(const void* ptr1, const void* ptr2, size_t len)
{
    unsigned char* c_ptr1 = (unsigned char*)ptr1;
    unsigned char* c_ptr2 = (unsigned char*)ptr2;

    int ret = 0;
    while (len-- && !ret) {
        ret = *(c_ptr1++) - *(c_ptr2++);
    }

    return ret;
}