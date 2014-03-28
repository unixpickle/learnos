#ifndef __LIBKERN_STDIO_H__
#define __LIBKERN_STDIO_H__

#include <shared/screen.h>

void print(const char * buffer);
void printHex(unsigned long number);
void die(const char * buffer);

#endif
