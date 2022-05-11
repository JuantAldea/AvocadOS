#ifndef __IO_H
#define __IO_H

#include <stdint.h>

// https://man7.org/linux/man-pages/man2/insb.2.html
// why ushort instead of uint16_t?

uint8_t insb(uint16_t port);
uint16_t insw(uint16_t port);

void outb(uint16_t port, uint8_t value);
void outw(uint16_t port, uint16_t value);

void io_delay(void);

#endif
