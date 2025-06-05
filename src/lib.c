#include "lib.h"
#include <stdarg.h>

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

char *digits = "0123456789ABCDEF";
void puthex(uint32_t number)
{
    int i;
    int started = 0;
    for (i = 28; i >= 0; i -= 4)
    {
        int k = (number >> i) & 0xF;
        if (k != 0 || started)
        {
            putc(digits[k]);
            started = 1;
        }
    }
    if (!started)
        putc('0');
}

void putdec(uint32_t number)
{
    if (number == 0)
    {
        putc('0');
        return;
    }

    char buffer[10];
    int i = 0;
    while (number > 0)
    {
        buffer[i++] = digits[number % 10];
        number /= 10;
    }
    while (i-- > 0)
    {
        putc(buffer[i]);
    }
}

void putbin(uint32_t number)
{
    if (number == 0)
    {
        putc('0');
        return;
    }

    char buffer[32];
    int i = 0;
    while (number > 0)
    {
        buffer[i++] = digits[number % 2];
        number >>= 1;
    }

    while (i-- > 0)
    {
        putc(buffer[i]);
    }
}

void printf(const char *fmt, ...)
{
    char c;
    va_list args;

    va_start(args, fmt);
    while ((c = *fmt) != '\0')
    {
        if (c == '%')
        {
            c = *(++fmt);
            switch (c)
            {
            case 'd':
                putdec(va_arg(args, uint32_t));
                break;
            case 'x':
                puthex(va_arg(args, uint32_t));
                break;
            case 'b':
                putbin(va_arg(args, uint32_t));
                break;
            case 's':
                puts(va_arg(args, char *));
                break;
            case 'c':
                putc(va_arg(args, int));
                break;
            case 'p':
                puthex((uint32_t)va_arg(args, void *));
                break;
            case '%':
                putc('%');
                break;
            default:
                break;
            }
        }
        else
        {
            putc(c);
        }
        fmt++;
    }
}
