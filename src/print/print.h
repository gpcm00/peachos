#ifndef PRINT_H
#define PRINT_H
#include "peachos.h"
void terminal_init();
void color_print(char* msg, char color);
void print(char* msg);
void print_raw_bytes(void* buffer, size_t len);

#endif