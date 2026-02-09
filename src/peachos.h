#ifndef PEACHOS_H
#define PEACHOS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define BIT(n)          (1<<(n))
#define N_BIT_MASK(n)   (BIT(n)-1)

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

#define PEACHOS_TOTAL_INTERRUPTS        512
#define KERNEL_CODE_SELECTOR            0x08
#define KERNEL_DATA_SELECTOR            0x10

#define PEACHOS_HEAP_SIZE_BYTES         104857600
#define PEACHOS_HEAP_BLOCK_SIZE         4096
#define PEACHOS_HEAP_ADDRESS            0x01000000 
#define PEACHOS_HEAP_TABLE_ADDRESS      0x00007E00
#define PEACHOS_SECTOR_SIZE             512

#define PEACHOS_MAX_PATH                108
#define PEACHOS_MAX_FILESYSTEMS         16
#define PEACHOS_MAX_FILE_DESCRIPTORS    512

enum Error_Code {
    EIO = 1,
    EINVARG,
    ENOMEM,
    EBADPATH,
    EFSNOTUS,
    ERDONLY,
    EUNIMP,
    EISTKN,
    EINFORMAT,
};

void panic(const char* msg);

#endif