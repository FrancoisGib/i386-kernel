#include "screen.h"

static uint16_t cursor_x = 0;
static uint16_t cursor_y = 0;

uint16_t get_color(uint8_t fg, uint8_t bg)
{
    return fg << 8 | bg;
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

void update_cursor()
{
    uint16_t pos = cursor_x + cursor_y * SCREEN_WIDTH;
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

void init_screen(void)
{
    enable_cursor();
    clear_screen();
}

void clear_screen(void)
{
    for (size_t y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (size_t x = 0; x < SCREEN_WIDTH; x++)
        {
            screen_putc(' ', x, y, 0xF);
        }
    }
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}

void putchar(char c)
{
    switch (c)
    {
    case '\n':
    {
        cursor_x = SCREEN_WIDTH - 1;
        break;
    }

    case '\b':
    {
        cursor_x--;
        screen_putc(' ', cursor_x, cursor_y, 0x0F);
        cursor_x--;
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
            clear_screen();
        }
    }
    update_cursor();
}