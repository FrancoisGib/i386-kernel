#include "keyboard.h"
#include "screen.h"
#include "ioport.h"

#define BUFFER_SIZE 128

#define SHIFT_PRESSED 0x2A
#define SHIFT_RELEASED 0xAA
#define RIGHT_SHIFT_PRESSED 0x36
#define RIGHT_SHIFT_RELEASED 0xB6
#define CTRL_PRESSED 0x1D
#define CTRL_RELEASED 0x9D
#define ALT_PRESSED 0x38
#define ALT_RELEASED 0xB8

#define SET_KEY(code, key) key_map[code] = key;
#define SET_EVENT_KEY(code, key)

unsigned char key_map[256];

char shift_pressed = 0;
char alt_pressed = 0;
char ctrl_pressed = 0;
char global_c = 0;

char handler = -1;
unsigned char keyboard_buffer[BUFFER_SIZE];

void init_key_map()
{
    for (int i = 0; i < 256; i++)
    {
        SET_KEY(i, '\0')
    }

    SET_KEY(0x02, '&')
    SET_KEY(0x04, '"')
    SET_KEY(0x05, '\'')
    SET_KEY(0x06, '(')
    SET_KEY(0x07, '-')
    SET_KEY(0x09, '_')
    SET_KEY(0x0C, ')')
    SET_KEY(0x0D, '=')

    SET_KEY(0x10, 'a')
    SET_KEY(0x11, 'z')
    SET_KEY(0x12, 'e')
    SET_KEY(0x13, 'r')
    SET_KEY(0x14, 't')
    SET_KEY(0x15, 'y')
    SET_KEY(0x16, 'u')
    SET_KEY(0x17, 'i')
    SET_KEY(0x18, 'o')
    SET_KEY(0x19, 'p')
    SET_KEY(0x1E, 'q')
    SET_KEY(0x1F, 's')
    SET_KEY(0x20, 'd')
    SET_KEY(0x21, 'f')
    SET_KEY(0x22, 'g')
    SET_KEY(0x23, 'h')
    SET_KEY(0x24, 'j')
    SET_KEY(0x25, 'k')
    SET_KEY(0x26, 'l')
    SET_KEY(0x27, 'm')
    SET_KEY(0x2C, 'w')
    SET_KEY(0x2D, 'x')
    SET_KEY(0x2E, 'c')
    SET_KEY(0x2F, 'v')
    SET_KEY(0x30, 'b')
    SET_KEY(0x31, 'n')

    SET_KEY(0x32, ',')
    SET_KEY(0x33, ';')
    SET_KEY(0x34, ':')
    SET_KEY(0x35, '!')
    SET_KEY(0x39, ' ')
    SET_KEY(0xE, '\b')
    SET_KEY(0x1C, '\n')

    SET_KEY(0xB, '0')
    SET_KEY(0x2, '1')
    SET_KEY(0x3, '2')
    SET_KEY(0x4, '3')
    SET_KEY(0x5, '4')
    SET_KEY(0x6, '5')
    SET_KEY(0x7, '6')
    SET_KEY(0x8, '7')
    SET_KEY(0x9, '8')
    SET_KEY(0xA, '9')
}

char get_char_from_code(unsigned char code)
{
    if (code == ALT_PRESSED)
    {
        alt_pressed = 1;
        return 0;
    }
    else if (code == ALT_RELEASED)
    {
        alt_pressed = 0;
        return 0;
    }
    else if (code == SHIFT_PRESSED)
    {
        shift_pressed = 1;
        return 0;
    }
    else if (code == SHIFT_RELEASED)
    {
        shift_pressed = 0;
        return 0;
    }
    else if (code == CTRL_PRESSED)
    {
        ctrl_pressed = 1;
        return 0;
    }
    else if (code == CTRL_RELEASED)
    {
        ctrl_pressed = 0;
        return 0;
    }
    else if (key_map[code] >= 'a' && key_map[code] <= 'z')
    {
        if (ctrl_pressed)
        {
            if (key_map[code] == 'l')
            {
                clear_screen();
            }
            ctrl_pressed = 0;
            return '\0';
        }

        if (shift_pressed)
        {
            return key_map[code] - 32;
        }
        else
        {
            return key_map[code];
        }
    }
    return key_map[code];
}

void keyboard_handler()
{
    unsigned char c = inb(0x60);
    c = get_char_from_code(c);
    if (c != 0)
    {
        static int cursor = 0;
        if (cursor >= BUFFER_SIZE)
        {
            cursor = 0;
        }
        keyboard_buffer[cursor] = c;
        cursor++;
        putchar(c);
    }
}

// unsigned char getc()
// {
//     static int cursor = 0;
//     if (cursor >= BUFFER_SIZE)
//     {
//         cursor = 0;
//     }
//     unsigned char c = keyboard_buffer[cursor];
//     keyboard_buffer[cursor % BUFFER_SIZE] = '\0';
//     cursor++;
//     return c;
// }

// void gets(char *buf, int nb_char)
// {
//     int i = 0;
//     while (i < nb_char)
//     {
//         buf[i] = getc();
//         i++;
//     }
// }