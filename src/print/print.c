#include "print.h"
#include <stdarg.h>

#define STACK_SIZE  32

#define VGA_WIDTH   80
#define VGA_HEIGHT  20
static uint16_t* video_mem = (uint16_t*)(0xB8000);
uint16_t terminal_row = 0;
uint16_t terminal_col = 0;

static inline void terminal_putchar(int x, int y, uint16_t c)
{
    video_mem[(y*VGA_WIDTH) + x] = c;
}

static char nibble_convert(char c)
{   
    char n = c & 0xF;
    if (n < 10) {
        return n + '0';
    }

    return n + 'A' - 10;
}

static void hex_convert(char c, uint16_t* hex)
{
    hex[0] |= nibble_convert(c);
    hex[1] |= nibble_convert(c >> 4);
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


static void put_uint2str(unsigned int n, uint16_t color)
{
    uint16_t stack[STACK_SIZE];
    size_t sp = 0;

    do {
        uint16_t c = (n % 10) + '0';
        stack[sp++] = color | c;        
        n /= 10;
    }  while (n);

    while (sp--) {
        terminal_writechar(stack[sp]);
    }
}


static void put_int2str(int n, uint16_t color)
{

    if (n < 0) {
        terminal_writechar(color | '-');
        n *= -1;
    }

    put_uint2str(n, color);
}


static void put_hex2str(unsigned int n, uint16_t color, size_t size)
{
    terminal_writechar(color | '0');
    terminal_writechar(color | 'x');
    
    uint16_t stack[STACK_SIZE];
    size_t sp = 0;

    uint16_t c[2];

    c[0] = color;
    c[1] = color;

    for (size_t i = 0; i < size; i++)
    {
        hex_convert(n & 0xFF, c);
        stack[sp++] = c[0];
        stack[sp++] = c[1];
        n >>= 8;
        c[0] &= 0xff00;
        c[1] &= 0xff00;
    }

    while (sp--) {
        terminal_writechar(stack[sp]);
    }
}

static void put_special(char c, uint16_t color, int n)
{
    switch (c)
    {
    case 'b':
        put_hex2str((unsigned int)n, color, 1);
        break;

    case 'w':
        put_hex2str((unsigned int)n, color, 2);
        break;

    case 'd':
        put_hex2str((unsigned int)n, color, 4);
        break;
    
    case 'i':
        put_int2str(n, color);
        break;
    
    case 'u':
        put_uint2str((unsigned int)n, color);
        break;
    
    default:
        terminal_writechar((uint16_t)c | color);
        break;
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

void color_print(const char* msg, char color)
{
    uint16_t base = color << 8;
    size_t len = strlen(msg);
    for (size_t i = 0; i < len; i++) {
        terminal_writechar(base | msg[i]);
    }
}

void puts(const char* msg)
{
    color_print(msg, 0x0F);
}

void print(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    uint16_t color = 0x0F00;    // blk bg + wh chars
    while (*msg != '\0')
    {
        if (*msg != '\\') {
            terminal_writechar(color | *(msg++));
        } else {
            msg++;
            int n = va_arg(args, int);
            put_special(*(msg++), color, n);
        }
    }
}

void print_raw_bytes(const void* buffer, size_t len)
{
    char* c_buffer = (char*)buffer;
    uint16_t out[2];
    out[0] = 0x0f00;
    out[1] = 0x0f00;

    uint16_t space = 0x00f00 | ' ';
    uint16_t newline = 0x0f00 | '\n';

    size_t i = 0;

    while (i < len) {
        hex_convert(c_buffer[i++], out);
        terminal_writechar(out[1]);
        terminal_writechar(out[0]);
        
        out[0] &= 0xff00;
        out[1] &= 0xff00;

        if (!(i % 16)) {
            terminal_writechar(newline);
        } else {
            terminal_writechar(space);
        }
    }
}

void print_newline()
{
    uint16_t newline = 0x0f00 | '\n';
    terminal_writechar(newline);
}