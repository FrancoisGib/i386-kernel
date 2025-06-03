#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <stddef.h>
#include <stdint.h>
#include "ioport.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define SCREEN_BASE 0xB8000
#define SCREEN ((uint16_t *)SCREEN_BASE)

#define CURSOR_SIZE 1
#define CURSOR_START 15
#define CURSOR_END (CURSOR_START + CURSOR_SIZE)

void putchar(char c);
void init_screen(void);

#endif // __SCREEN_H__