#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <stddef.h>
#include <stdint.h>

typedef uint64_t useconds_t;

int brk(const void * addr);
void * sbrk(intptr_t increment);
void _exit(int unused);
unsigned int sleep(unsigned int secs);
int usleep(useconds_t time);

#endif
