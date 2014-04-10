#include "include/anmalloc_bindings.h"
#include <unistd.h>

void * anmalloc_sbrk(intptr_t incr) {
  return sbrk(incr);
}

int anmalloc_brk(const void * addr) {
  return brk(addr);
}

void anmalloc_lock(anmalloc_lock_t * lock) {
}

void anmalloc_unlock(anmalloc_lock_t * lock) {
}

