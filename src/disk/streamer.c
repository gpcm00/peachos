#include "streamer.h"

struct disk_streamer diskstreamer_new(int disk_id)
{
    struct disk_streamer ret;

    ret.disk = get_disk(disk_id);
    ret.pos = 0;

    return ret;
}

int diskstreamer_seek(struct disk_streamer* streamer, int pos)
{
    streamer->pos = pos;
    return 0;
}

int diskstreamer_read(struct disk_streamer* streamer, void* out, size_t total)
{
    char buffer[PEACHOS_SECTOR_SIZE];

    size_t current_offset = streamer->pos;
    bool overflow = ((current_offset + total) >= PEACHOS_SECTOR_SIZE);

    int lba = current_offset / PEACHOS_SECTOR_SIZE;
    int ret = disk_read_block(streamer->disk, lba, 1, buffer);
    if (ret < 0) {
        goto Fail_Out;
    }

    size_t read_offset = current_offset % PEACHOS_SECTOR_SIZE;
    size_t read_size = (overflow)? PEACHOS_SECTOR_SIZE - read_offset : total;

    char* c_out = (char*)out;
    size_t end_pos = read_offset + read_size;
    for (size_t i = read_offset; i < end_pos; i++) {
        *(c_out++) = buffer[i];
        streamer->pos++;
    }
    
    if (overflow) {
        ret = diskstreamer_read(streamer, c_out, total - read_size);
    }

Fail_Out:
    return ret;
}