#include "print.h"
#include <stddef.h>
#include <stdint.h>

#define VGA_WIDTH   80
#define VGA_HEIGHT  20
static uint16_t* video_mem = (uint16_t*)(0xB8000);
uint16_t terminal_row = 0;
uint16_t terminal_col = 0;

static inline void terminal_putchar(int x, int y, uint16_t c)
{
    video_mem[(y*VGA_WIDTH) + x] = c;
}

static size_t strlen(const char* str)
{
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    
    return len;
}

static void terminal_writechar(uint16_t c)
{
    if ((char)c == '\n') {
        terminal_col += VGA_WIDTH;     
    } else {
        terminal_putchar(terminal_col, terminal_row, c);
        terminal_col++;
    }

    if (terminal_col >= VGA_WIDTH) {
        terminal_col = 0;
        terminal_row++;

        if (terminal_row >= VGA_HEIGHT) {
            terminal_row = 0;
        }
    }
}

void terminal_init()
{
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            video_mem[(y*VGA_WIDTH) + x] = (uint16_t)(' ');
        }
    }
}

void color_print(char* msg, char color)
{
    uint16_t base = color << 8;
    size_t len = strlen(msg);
    for (size_t i = 0; i < len; i++) {
        terminal_writechar(base | msg[i]);
    }
}

void print(char* msg)
{
    color_print(msg, 0x0F);
}