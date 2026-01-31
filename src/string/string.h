#ifndef STRING_H
#define STRING_H
#include "peachos.h"

size_t strlen(const char* str);
size_t strnlen(const char* str, size_t max);
bool isdigit(const char c);
int tonumericdigit(const char c);

#endif
