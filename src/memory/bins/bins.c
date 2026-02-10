#include "bins.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"

#define MIN_BIN_SLACK   2
#define SIGNATURE_HASH  0x8421137F

enum BINS_ENTRY {
    FREE_LIST,
    TAKEN_BLOCK,
};

struct free_list_header {
    void* next;
    size_t bins;
    uint8_t entry;
} __attribute__((packed));

struct taken_block_header {
    uint32_t signature;
    size_t bins;
    uint8_t entry;
} __attribute__((packed));

static uint32_t align_to_page_size(size_t addr)
{
    return (addr + (PEACHOS_HEAP_BLOCK_SIZE-1)) & ~((PEACHOS_HEAP_BLOCK_SIZE-1));
}

static uint32_t aligned(uint32_t val, uint32_t alignemnt)
{
    return val & ~(alignemnt-1);
}

static inline struct free_list_header* aligned_free_list(void* addr, uint32_t alignment)
{
    return (struct free_list_header*)aligned((uint32_t)addr, alignment);
}

static inline struct taken_block_header* aligned_taken_block_header(void* addr, uint32_t alignment)
{
    return (struct taken_block_header*)aligned((uint32_t)addr, alignment);
}

static uint32_t align_to(uint32_t val, uint32_t alignemnt)
{
    return (val + (alignemnt-1)) & ~(alignemnt-1);
}

static bool validate_alignment(uint32_t alignment)
{
    return (alignment != 0) && ((alignment & (alignment-1)) == 0);
}

static bool validate_bin_size(uint32_t bin_size)
{
    uint32_t min_size = align_to(sizeof(struct free_list_header), sizeof(uint32_t));
    return (bin_size > min_size) && validate_alignment(bin_size);
}

void* get_next_bin(void* addr, uint32_t bin_size)
{
    uint32_t ret = aligned((uint32_t)addr, bin_size) + bin_size;
    return (void*)ret;
}

void* get_previous_bin(void* addr, uint32_t bin_size)
{
    uint32_t ret = aligned((uint32_t)addr, bin_size) - bin_size;
    return (void*)ret;   
}

static struct free_list_header* make_free_list_header(void* addr, uint32_t n_bins, uint32_t bin_size, void* next)
{
    struct free_list_header* base = aligned_free_list(addr, bin_size);
    base->next = next;
    base->bins = n_bins;
    base->entry = FREE_LIST;

    return base;
}

static uint32_t hash_signature(struct taken_block_header* addr, uint32_t bin_size)
{
    uint32_t hash = (uint32_t)addr + ((addr->bins) * bin_size);
    return (hash) ^ SIGNATURE_HASH;
}

static bool validate_hash(struct taken_block_header* addr, uint32_t bin_size)
{
    uint32_t hash = hash_signature(addr, bin_size);
    return (addr->signature == hash);
}

static struct taken_block_header* make_taken_block_header(void* addr, uint32_t n_bins, uint32_t bin_size)
{
    struct taken_block_header* base = aligned_taken_block_header(addr, bin_size);
    base->bins = n_bins;
    base->entry = TAKEN_BLOCK;
    base->signature = hash_signature(base, bin_size);
    return base;
}

struct bins_memory* bins_alloc_memory(size_t size, uint32_t bin_size)
{
    if (!validate_bin_size(bin_size) || (size < (MIN_BIN_SLACK * bin_size))) {
        return NULL;
    }

    size = align_to_page_size(size);
    struct bins_memory* ret = (struct bins_memory*)kzalloc(size);

    uint32_t available_bins = (size / bin_size) - 1;        // -1 because of the current bin

    ret->bin_size = bin_size;
    ret->next = get_next_bin(ret, ret->bin_size);

    make_free_list_header(ret->next, available_bins - 1, ret->bin_size, NULL);    // minus free_list_header bin

    return ret;
}

static void* skip_n_bins(void* addr, uint32_t n_bins, uint32_t bin_size)
{
    uint32_t next = aligned((uint32_t)addr, bin_size) + n_bins * bin_size;
    return (void*)next;
}

struct free_list_header* allocate_free_blocks(void* addr, uint32_t n_bins, uint32_t bin_size)
{
    struct free_list_header* base = aligned_free_list(addr, bin_size);

    if (n_bins <= (base->bins - MIN_BIN_SLACK)) {
        void* first_bin = get_next_bin(base, bin_size);     // count from the first available bin, not the header
        struct free_list_header* new_free_list = skip_n_bins(first_bin, n_bins, bin_size);
        uint32_t available_bins = base->bins - n_bins;

        return make_free_list_header(new_free_list, available_bins - 1, bin_size, base->next);
    }

    return base->next;
}

struct free_list_header* _alloc_data(struct free_list_header* header, uint32_t n_bins, uint32_t bin_size, void** ret)
{
    if (header == NULL) {
        *ret = NULL;
        return NULL;
    }
    
    if (header->bins >= n_bins) {
        void* next = allocate_free_blocks(header, n_bins, bin_size);
        make_taken_block_header(header, n_bins, bin_size);
        *ret = get_next_bin(header, bin_size);
        return next;
    } else {
        header->next = _alloc_data(header->next, n_bins, bin_size, ret);
    }

    return header->next;
}

void* bins_alloc_data(struct bins_memory* bin, size_t size) 
{
    void* ret = NULL;
    uint32_t n_bins = align_to(size, bin->bin_size) / bin->bin_size;
    bin->next = _alloc_data(bin->next, n_bins, bin->bin_size, &ret);

    return ret;
}

struct taken_block_header* get_taken_block_header(void* ptr, uint32_t bin_size)
{
    struct taken_block_header* header = get_previous_bin(ptr, bin_size);
    if (header->entry != TAKEN_BLOCK || !validate_hash(header, bin_size)) {
        return NULL;
    }

    return header;
}

static bool compare_addr(void* p1, void* p2)
{
    return (uint32_t)p1 < (uint32_t)p2;
}

void* merge_free_list_header(struct free_list_header* header, uint32_t bin_size)
{
    void* next = get_next_bin(header, bin_size);
    struct free_list_header* free_list = skip_n_bins(next, header->bins, bin_size);
    if (free_list == header->next) {
        header->bins += free_list->bins + 1;        // + 1 for the free list header
        header->next = free_list->next;
    }

    return header;
}

void* _dealloc_data(struct free_list_header* header, struct taken_block_header* ptr, uint32_t bin_size)
{
    if (compare_addr(ptr, header)) {
        header = make_free_list_header(ptr, ptr->bins, bin_size, header);
    } else {
        header->next = _dealloc_data(header->next, ptr, bin_size);
    }

    return merge_free_list_header(header, bin_size);
}

int bins_dealloc_data(struct bins_memory* bin, void* ptr)
{
    struct taken_block_header* taken_header = get_taken_block_header(ptr, bin->bin_size);
    if (taken_header == NULL) {
        return -EINVARG;
    }

    bin->next = _dealloc_data(bin->next, taken_header, bin->bin_size);

    return 0;
}

void bins_destroy_memory(void* bin)
{
    
}