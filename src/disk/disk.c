#include "disk.h"
#include "io/io.h"

#define BYTE(n)    ((n) & 0xFF)

int disk_read_sector(int lba, size_t total, void* buf)
{
    outb(0x1F6, BYTE(lba >> 24) | 0xE0);    // master mode
    outb(0x1F2, total);         // total sectors to read
    
    // send remaining bytes
    outb(0x1F3, BYTE(lba));
    outb(0x1F4, BYTE(lba >> 8));
    outb(0x1F5, BYTE(lba >> 16));

    // read sector command
    outb(0x1F7, 0x20);

    uint16_t* ptr = (uint16_t*) buf;

    for (size_t i = 0; i < total; i++) {
        while (!(insb(0x1F7) & 0x08));

        for (size_t i = 0; i < 256; i++) {
            *ptr = insw(0x1F0);
            ptr++;
        }
    }

    return 0;
}