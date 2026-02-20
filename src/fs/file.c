#include "file.h"
#include "memory/memory.h"

#define LEN(arr)    (sizeof(arr)/sizeof(arr[0]))

struct file_descriptor_info
{
    struct file_descriptor file_descriptor;
    bool taken;
};


struct filesystem* filesystems[PEACHOS_MAX_FILESYSTEMS];
struct file_descriptor_info file_descriptors[PEACHOS_MAX_FILE_DESCRIPTORS];


static struct filesystem** fs_get_free_filesystem()
{
    for (int i = 0; i < LEN(filesystems); i++) {
        if (filesystems[i] == NULL) {
            return &filesystems[i];
        }
    }

    return NULL;
}
int fs_insert_filesystem(struct filesystem* filesystem)
{
    struct filesystem** fs = fs_get_free_filesystem();
    if (!fs) {
        return -ENOMEM;
    }

    *fs = filesystem;

    return 0;
}
static void fs_static_load() 
{
    // fs_insert_filesystem(fat16_init());
}
void fs_load()
{
    fs_static_load();
}
void fs_init()
{
    memset(filesystems, 0, sizeof(filesystems));
    memset(file_descriptors, 0, sizeof(file_descriptors));
    fs_load();
}

static int file_new_descriptor(struct file_descriptor* desc_out)
{
    for (int i = 0; i < LEN(file_descriptors); i++) {
        if (!file_descriptors[i].taken) {
            file_descriptors[i].taken = true;
            file_descriptors[i].file_descriptor.index = i + 1;
            desc_out = &file_descriptors[i].file_descriptor;
            return 0;
        }
    }

    return -ENOMEM;
}

static struct file_descriptor* file_descriptor_fd(int fd)
{
    if (fd <= 0 || fd > LEN(file_descriptors)) {
        return NULL;
    }

    int index = fd - 1;
    if (file_descriptors[index].taken) {
        return &file_descriptors[index].file_descriptor;
    }

    return NULL;
}

struct filesystem* fs_resolve(struct disk* disk)
{
    for (int i = 0; i < LEN(filesystems); i++) {
        if ((filesystems[i] != NULL) && (filesystems[i]->resolve(disk) == 0)) {
            return filesystems[i];
        }
    }

    return NULL;
}


int fopen(const char* filename, const char* mode)
{
    return -EIO;
}