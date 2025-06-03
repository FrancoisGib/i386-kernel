#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define UNUSED __attribute__((unused))

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define SCREEN_BASE 0xB8000
#define SCREEN ((uint16_t *)SCREEN_BASE)

#define CURSOR_SIZE 1
#define CURSOR_START 15
#define CURSOR_END (CURSOR_START + CURSOR_SIZE)

static uint16_t cursor_x = 0;
static uint16_t cursor_y = 0;

static inline unsigned char inb(unsigned short port UNUSED)
{
    unsigned char ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(unsigned short port UNUSED, unsigned char value UNUSED)
{
    __asm__("outb %0,%1" ::"a"(value), "Nd"(port));
}

size_t strlen(const char *str)
{
    size_t len = 0;
    while (str[len])
    {
        len++;
    }
    return len;
}

void enable_cursor(void)
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | CURSOR_START);
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | CURSOR_END);
}

void disable_cursor(void)
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void set_cursor(int x, int y)
{
    uint16_t pos = x + y * SCREEN_WIDTH;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void screen_putc(char c, int x, int y, uint8_t color)
{
    uint16_t pos = x + y * SCREEN_WIDTH;
    SCREEN[pos] = (color << 8) | c;
}

static inline void init_screen(void)
{
    for (size_t y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (size_t x = 0; x < SCREEN_WIDTH; x++)
        {
            screen_putc(' ', x, y, 0xF);
        }
    }
    enable_cursor();
}

void putc(char c)
{
    switch (c)
    {
    case '\n':
    {
        cursor_x = SCREEN_WIDTH - 1;
        break;
    }

    default:
        screen_putc(c, cursor_x, cursor_y, 0x0F);
        break;
    }

    if (++cursor_x == SCREEN_WIDTH)
    {
        cursor_x = 0;
        if (++cursor_y == SCREEN_HEIGHT)
        {
            cursor_y = 0;
        }
    }
    set_cursor(cursor_x, cursor_y);
}

void puts(const char *data)
{
    for (size_t i = 0; i < strlen(data); i++)
    {
        putc(data[i]);
    }
}

void main(void)
{
    init_screen();
    puts("Hello World !");
}
