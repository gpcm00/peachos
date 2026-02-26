#include "fat16.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "memory/bins/bins.h"

#define PEACHOS_FAT16_SIGNATURE     0x29
#define PEACHOS_FAT16_ENTRY_SIZE    0x02
#define PEACHOS_FAT16_BAD_SECTOR    0x0FF7
#define PEACHOS_FAT16_UNUSED        0x00

typedef unsigned int FAT_ITEM_TYPE;
enum FAT_ITEM_TYPE_SYMBOLS {
    FAT_ITEM_TYPE_DIRECTORY,
    FAT_ITEM_TYPE_FILE,

    NUM_OF_FAT_ITEM_TYPES,
};

#define FAT16_SIGNATURE             0x29    // from fat16 specification

#define FAT16_FILE_READ_ONLY        BIT(0)
#define FAT16_FILE_HIDDEN           BIT(1)
#define FAT16_FILE_SYSTEM           BIT(2)
#define FAT16_FILE_VOLUME_LABEL     BIT(3)
#define FAT16_FILE_SUBDIRECTORY     BIT(4)
#define FAT16_FILE_ARCHIVED         BIT(5)
#define FAT16_FILE_DEVICE           BIT(6)
#define FAT16_FILE_RESERVED         BIT(7)

struct fat_header {
    uint8_t short_jmp_ins[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entries;
    uint16_t number_of_sectors;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t sectors_big;
} __attribute__((packed));

struct fat_header_extended {
    uint8_t drive_number;
    uint8_t win_nt_bit;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_id_string[11];
    uint8_t system_id_string[8];
} __attribute__((packed));

struct fat_h {
    struct fat_header primary_header;
    union fat_h_e {
        struct fat_header_extended extended_header;
    } shared;
} __attribute__((packed));

struct fat_directory_item {
    uint8_t filename[8];
    uint8_t ext[3];
    uint8_t attribute;
    uint8_t reserved;
    uint8_t creation_time_tenths_of_a_sec;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access;
    uint16_t high16_bits_first_cluster;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t low16_bits_first_cluster;
    uint32_t filesize;
} __attribute__((packed));

struct fat_directory {
    struct fat_directory_item* item;
    int total;
    int sector_pos;
    int ending_sector_pos;
};

struct fat_item {
    union
    {
        struct fat_directory_item* item;
        struct fat_directory* directory;
    };

    FAT_ITEM_TYPE type;
};

struct fat_item_descriptor {
    struct fat_item* item;
    uint32_t pos;
};

struct fat_private {
    struct fat_h header;
    struct fat_directory root;

    // stream data clusters
    struct disk_streamer cluster_read_stream;

    // stream file allocation table
    struct disk_streamer fat_read_stream;

    // stream directory
    struct disk_streamer directory_read_stream;
};

static void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode);
static int fat16_resolve(struct disk* disk);

const struct filesystem fat16_fs_info = {
    .name =     "FAT16",
    .open =     &fat16_open,
    .resolve =  &fat16_resolve
};

static struct bins_memory* fat16_heap = NULL;
static struct filesystem* fat16_fs = NULL;

struct filesystem* fat16_init()
{
    struct filesystem* ret = NULL;
    fat16_heap = bins_alloc_memory(PEACHOS_HEAP_BLOCK_SIZE, 16);
    if (!fat16_heap) {
        goto Out;
    }

    ret = bins_alloc_data(fat16_heap, sizeof(struct filesystem));
    if (!ret) {
        bins_destroy_memory(fat16_heap);
        goto Out;
    }

    memcpy(ret, &fat16_fs_info, sizeof(struct filesystem));
    fat16_fs = ret;

Out:
    return ret;
}

static void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode)
{
    return NULL;
}

static void fat16_init_fat_private(struct disk* disk, struct fat_private* private)
{
    memset(private, 0, sizeof(struct fat_private));
    private->directory_read_stream = diskstreamer_new(disk->disk_id);
    private->cluster_read_stream = diskstreamer_new(disk->disk_id);    
    private->fat_read_stream = diskstreamer_new(disk->disk_id); 
}

static int_least64_t fat16_sector_to_absolute(struct disk* disk, int sector)
{
    return disk->sector_size * sector;
}

static int fat16_get_total_items_for_dir(struct disk* disk, struct fat_private* private, uint32_t start)
{
    struct fat_directory_item item;
    struct fat_directory_item empty_item;
    memset(&item, 0, sizeof(item));
    memset(&empty_item, 0, sizeof(empty_item));

    // struct fat_private* private = &disk->private_data;

    int i = 0;
    int start_pos = start * disk->sector_size;
    struct disk_streamer* stream = &private->directory_read_stream;
    if (diskstreamer_seek(stream, start_pos) > 0) {
        return -EIO;
    }

    while (1) {
        if (diskstreamer_read(stream, &item, sizeof(item)) < 0) {
            return -EIO;
        }

        if (item.filename[0] == 0x00) {
            break;
        }

        if (item.filename[0] == 0xE5) {
            continue;
        }

        i++;
    }

    return i;
}

static int fat16_get_root_directory(struct disk* disk, struct fat_private* private, struct fat_directory* directory)
{
    int res = 0;
    struct fat_directory_item* dir = NULL;
    struct fat_header* primary_header = &private->header.primary_header;
    int root_pos = primary_header->fat_copies * primary_header->sectors_per_fat + primary_header->reserved_sectors;
    int root_entries = primary_header->root_dir_entries;
    int root_size = root_entries * sizeof(struct fat_directory_item);
    int total_sectors = root_size / disk->sector_size;
    total_sectors += (root_size % disk->sector_size)? 1 : 0;

    // get total items
    int total_items = fat16_get_total_items_for_dir(disk, private, root_pos);
    if (total_items < 0) {
        res = -EIO;
        goto Out;
    }

    // seek to abosulote root pos
    struct disk_streamer* stream = &private->directory_read_stream;
    if (diskstreamer_seek(stream, fat16_sector_to_absolute(disk, root_pos)) < 0) {
        res = -EIO;
        goto Out;
    }

    // read from abs offst with tottal items
    dir = kmalloc(root_size);   // allocate with kmalloc because we may use multiple pages
    if (!dir) {
        res = -ENOMEM;
        goto Out;
    }

    if (diskstreamer_read(stream, dir, root_size) < 0) {
        kfree(dir);
        res = -EIO;
        goto Out;
    }    

    directory->item = dir;
    directory->total = total_items;
    directory->sector_pos = root_pos;
    directory->ending_sector_pos = root_pos + total_sectors;

Out:
    return res;
    
}

static int fat16_resolve(struct disk* disk)
{
    if (!disk) {
        return -EINVARG;
    }

    int res = 0;

    struct fat_private* private = bins_alloc_data(fat16_heap, sizeof(struct fat_private));
    if (!private) {
        return -ENOMEM;
    }

    fat16_init_fat_private(disk, private);

    struct disk_streamer stream = diskstreamer_new(disk->disk_id);
    if (diskstreamer_read(&stream, &private->header, sizeof(struct fat_h)) < 0) {
        res = -EIO;
        goto Error_Out;
    }

    if (private->header.shared.extended_header.signature != FAT16_SIGNATURE) {
        res = -EFSNOTUS;
        goto Error_Out;
    }

    res = fat16_get_root_directory(disk, private, &private->root);
    if (res < 0) {
        goto Error_Out;
    }

    disk->private_data = private;
    disk->filesystem = fat16_fs;

    return 0;

Error_Out:
    bins_dealloc_data(fat16_heap, private);
    return res;
}


