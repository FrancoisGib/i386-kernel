#ifndef __LIB_H__
#define __LIB_H__

#include <stddef.h>
#include "screen.h"

void putc(char c);
void puts(const char *data);
size_t strlen(const char *str);

#endif // __LIB_H__