#include <stdint.h>
#include <stddef.h>
#include <lock.h>

typedef basic_lock_t anmalloc_lock_t;
#define ANMALLOC_LOCK_INITIALIZER BASIC_LOCK_INITIALIZER

void * anmalloc_sbrk(intptr_t incr);
int anmalloc_brk(const void * addr);

void anmalloc_lock(anmalloc_lock_t * lock);
void anmalloc_unlock(anmalloc_lock_t * lock);

