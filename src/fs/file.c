#include "file.h"
#include "memory/memory.h"
#include "scrap/scrap.h"

struct filesystem* filesystems[PEACHOS_MAX_FILESYSTEMS];
struct file_descriptor* file_descriptors[PEACHOS_MAX_FILE_DESCRIPTORS];

struct scrap_memory* scrap;

static struct filesystem** fs_get_free_filesystem();
void fs_insert_filesystem(struct filesystem* filesystem);
static void fs_static_load() 
{
    return;
}
void fs_load();
void fs_init();
void fs_insert_filesystem(struct filesystem* filesystem);
struct filesystem* fs_resolve(struct disk* disk);


int fopen(const char* filename, const char* mode)
{
    return -EIO;
}