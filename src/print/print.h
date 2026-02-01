#ifndef PRINT_H
#define PRINT_H
#include "peachos.h"
void terminal_init();
void color_print(const char* msg, char color);
void print(const char* msg);
void print_raw_bytes(const void* buffer, size_t len);
void print_newline();

#endif