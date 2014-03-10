#ifndef __LIBKERN_STDIO_H__
#define __LIBKERN_STDIO_H__

#include <shared/screen.h>

void print(const char * buffer);
void printHex(unsigned long number);
void die(const char * buffer);

void print_initialize();
void print_lock();
void print_unlock();

#endif
