#ifndef FMALLOC_H
#define FMALLOC_H

#include "peachos.h"

struct fmalloc_heap {
    void* base;
};

int fmalloc_alloc_heap(struct fmalloc_heap* mem, size_t size);
void fmalloc_dealloc_heap(struct fmalloc_heap* mem);
void* fmalloc_alloc(struct fmalloc_heap* mem, size_t size);
void fmalloc_free(struct fmalloc_heap* mem, void* ptr);

#endif
