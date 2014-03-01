#ifndef __LIBKERN_STDIO_H__
#define __LIBKERN_STDIO_H__

#include <shared/screen.h>

void print64(const char * buffer);
void printHex64(unsigned long number);
void die(const char * buffer);

#endif