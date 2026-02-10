#include "fmalloc.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"

#define ALIGN(strct)    align(sizeof(strct))  
#define SIGNATURE_HASH  0x8421137F

typedef uint32_t FMALLOC_HEAP_ENTRY;
typedef uint32_t FMALLOC_HEAP_SIGNATURE;

enum FMALLOC_HEAP_ENTRY_VALUES {
    FMALLOC_HEAP_FREE_LIST,
    FMALLOC_HEAP_BLOCK,
};

struct fmalloc_heap_header {
    struct free_list* first;
    size_t total;
} __attribute__((packed));

struct free_list {
    FMALLOC_HEAP_ENTRY entry;
    size_t available;
    struct free_list* next;
} __attribute__((packed));

struct fmalloc_block {
    FMALLOC_HEAP_ENTRY entry;
    size_t available;
} __attribute__((packed));

struct fmalloc_termination {
    struct fmalloc_block* header;
    FMALLOC_HEAP_SIGNATURE signature;
};

static bool validate_alignment(uint32_t alignment)
{
    return (alignment != 0) && ((alignment & (alignment-1)) == 0);
}

static uint32_t align(size_t addr)
{
    return (addr + N_BIT_MASK(2)) & ~(N_BIT_MASK(2));
}

static uint32_t aligned(size_t addr)
{
    return addr & ~N_BIT_MASK(2);
}

static struct free_list* initial_free_list(void* base, size_t size)
{
    uint32_t free_list_base = (uint32_t)base + sizeof(struct fmalloc_heap_header);
    return (struct free_list*)align(free_list_base);
}

static size_t inital_free_size(size_t size)
{
    size = size - sizeof(struct fmalloc_heap_header);
    return aligned(size);
}

static struct free_list* init_free_list(void* base, size_t size)
{
    struct free_list* first = initial_free_list(base, size);
    
    first->next = NULL;
    first->entry = FMALLOC_HEAP_FREE_LIST;
    first->available = inital_free_size(size);

    return first;
}

static void init_heap_header(void* base, size_t size)
{
    struct fmalloc_heap_header* header = (struct fmalloc_heap_header*)base;
    header->total = inital_free_size(size);
    header->first = init_free_list(base, size);
}

int fmalloc_alloc_heap(struct fmalloc_heap* mem, size_t n)
{
    void* base = kzalloc(n * PEACHOS_HEAP_BLOCK_SIZE);
    if (!base) {
        return -ENOMEM;
    }

    mem->base = base;
    size_t size = n * PEACHOS_HEAP_BLOCK_SIZE;

    init_heap_header(mem->base, size);

    return 0;
}

void fmalloc_dealloc_heap(struct fmalloc_heap* mem)
{
    kfree(mem->base);
    mem->base = NULL;
}

static size_t actual_block_size(size_t size)
{
    // every block has a hidden header at the top and a signature at the end
    size_t meta_data = align(sizeof(struct fmalloc_block)) + align(sizeof(struct fmalloc_termination));
    return meta_data + align(size);
}

static inline FMALLOC_HEAP_SIGNATURE sign_address(void* addr)
{
    return (FMALLOC_HEAP_SIGNATURE)addr ^ SIGNATURE_HASH;
}

static void sign_taken_block(struct fmalloc_block* header, void* base, size_t size)
{
    uint32_t signature_addr = (uint32_t)base + size;
    struct fmalloc_termination* termination = (struct fmalloc_termination*)align(signature_addr);
    termination->header = header;
    termination->signature = sign_address(&termination->signature);
}

static size_t min_block_slack(size_t size) 
{
    // minimum block should be able to allocate at least 4 free lists
    size_t slack = 4 * sizeof(struct free_list);
    return actual_block_size(size) + align(slack);
}


static void* create_taken_block(void* base, size_t size)
{
    struct fmalloc_block* block = (struct fmalloc_block*)base;
    block->available = align(size);
    block->entry = FMALLOC_HEAP_BLOCK;

    uint32_t ret = (uint32_t)block + align(sizeof(struct fmalloc_block));
    void* ret_addr = (void*)align(ret);

    sign_taken_block(block, ret_addr, size);

    return ret_addr;
}

static struct free_list* get_next_block(struct free_list* mem, size_t size)
{
    uint32_t base = (uint32_t)mem + actual_block_size(size);
    return (struct free_list*)align(base);
}

static struct free_list* divide_bin(struct free_list* mem, size_t size)
{
    if (mem->available < min_block_slack(size)) {
        return mem->next;
    }

    size_t available = mem->available - actual_block_size(size);

    struct free_list* next = get_next_block(mem, size);
    next->next = mem->next;
    next->entry = FMALLOC_HEAP_FREE_LIST;
    next->available = align(available);
    
    return next;
}

static struct free_list* _fmalloc_alloc(struct free_list* mem, size_t size, void** ret)
{
    if (mem == NULL) {
        *ret = NULL;
        return NULL;
    }

    if (mem->available >= actual_block_size(size)) {
        struct free_list* tmp = mem;
        mem = divide_bin(mem, size);
        *ret = create_taken_block(tmp, size);
        // sign_taken_block(*ret, size);
    } else {
        mem->next = _fmalloc_alloc(mem->next, size, ret);
    }

    return mem;
}

void* fmalloc_alloc(struct fmalloc_heap* mem, size_t size)
{
    struct fmalloc_heap_header* base = (struct fmalloc_heap_header*)mem->base;
    void* ret = NULL;
    base->first = _fmalloc_alloc(base->first, align(size), &ret);

    return ret;
}

static struct fmalloc_block* get_block(void* ptr)
{
    uint32_t base = (uint32_t)ptr - sizeof(struct fmalloc_block);
    return (struct fmalloc_block*)aligned(base);
}

static struct free_list* change_to_free_list(struct fmalloc_block* block, struct free_list* next)
{
    size_t available = block->available;
    struct free_list* ret = (struct free_list*)block;
    ret->available = available + actual_block_size(available);
    ret->entry = FMALLOC_HEAP_FREE_LIST;
    ret->next = next;
    return ret;
}

static void merge_free_list(struct free_list* mem)
{

}

static struct free_list* realloc_free_list(struct free_list* mem, void* ptr)
{
    struct fmalloc_block* head = get_block(ptr);
    return change_to_free_list(head, mem);
}

static struct free_list* _fmalloc_free(struct free_list* mem, void* ptr)
{
    if ((uint32_t)ptr > (uint32_t)mem) {
        mem->next = _fmalloc_free(mem->next, ptr);
    } else {
        mem = realloc_free_list(mem, ptr);
    }

    merge_free_list(mem);
}

void fmalloc_free(struct fmalloc_heap* mem, void* ptr)
{
    mem->base = _fmalloc_free(mem->base, ptr);
}