#include <pthread.h>
#include <stdint.h>

typedef pthread_mutex_t anmalloc_lock_t;
#define ANMALLOC_LOCK_INITIALIZER PTHREAD_MUTEX_INITIALIZER;

void * anmalloc_sbrk(intptr_t incr);
int anmalloc_brk(const void * addr);
void anmalloc_lock(anmalloc_lock_t * lock);
void anmalloc_unlock(anmalloc_lock_t * lock);

// extra functions
uint64_t __anmalloc_brk_size();
