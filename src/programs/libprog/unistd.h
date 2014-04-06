#include <stddef.h>
#include <stdint.h>

typedef uint64_t useconds_t;

int brk(void * addr);
void * sbrk(intptr_t increment);
void _exit(int unused);
void exit(int unused);
unsigned int sleep(unsigned int secs);
int usleep(useconds_t time);

