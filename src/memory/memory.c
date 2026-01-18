#include "memory.h"

void* memset(void* ptr, int c, size_t len)
{
    char* c_ptr = (char*)ptr;
    for (size_t i = 0; i < len; i++) {
        *(c_ptr++) = (char)c;
    }

    return ptr;
}