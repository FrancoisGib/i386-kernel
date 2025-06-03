#ifndef __IOPORT_H__
#define __IOPORT_H__

#define UNUSED __attribute__((unused))

static inline unsigned char inb(unsigned short port UNUSED)
{
    unsigned char ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(unsigned short port UNUSED, unsigned char value UNUSED)
{
    __asm__ volatile("outb %0, %1" ::"a"(value), "Nd"(port));
}

#endif // __IOPORT_H__