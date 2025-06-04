#include "lib.h"

void putc(char c)
{
    putchar(c);
}

void puts(const char *str)
{
    for (size_t i = 0; i < strlen(str); i++)
    {
        putc(str[i]);
    }
}

size_t strlen(const char *str)
{
    size_t len = 0;
    while (str[len] != '\0')
    {
        len++;
    }
    return len;
}

void *memset(void *ptr, int value, size_t size)
{
    unsigned char *p = (unsigned char *)ptr;
    unsigned char val = (unsigned char)value;
    for (size_t i = 0; i < size; i++)
    {
        p[i] = val;
    }
    return ptr;
}