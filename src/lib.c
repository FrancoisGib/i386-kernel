#include "lib.h"

void putc(char c)
{
    putchar(c);
}

void puts(const char *data)
{
    for (size_t i = 0; i < strlen(data); i++)
    {
        putc(data[i]);
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