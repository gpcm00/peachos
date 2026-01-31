#include "string.h"

size_t strlen(const char* str)
{
    size_t i = 0;
    while (str[i] != '\0') {
        i++;
    }

    return i;
}

size_t strnlen(const char* str, size_t max)
{
    size_t i;
    for (i = 0; i < max && str[i] != '\0'; i++);
    return i;
}

bool isdigit(const char c)
{
    return c >= '0' && c <= '9';
}

int tonumericdigit(char c)
{
    return c - '0';
}