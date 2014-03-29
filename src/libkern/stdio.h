#ifndef __LIBKERN_STDIO_H__
#define __LIBKERN_STDIO_H__

#include <shared/screen.h>
#include <stdint.h>

void print(const char * buffer);
void printColor(uint8_t color);
void printHex(unsigned long number);
void die(const char * buffer);

#endif
