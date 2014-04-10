#include "include/anmalloc_bindings.h"
#include <unistd.h>

void * anmalloc_sbrk(intptr_t incr) {
  printf("sbrk()...");
  void * ret = sbrk(incr);
  printf(" returning %x\n", ret);
  return ret;
}

int anmalloc_brk(const void * addr) {
  return brk(addr);
}

void anmalloc_lock(anmalloc_lock_t * lock) {
  basic_lock_lock(lock);
}

void anmalloc_unlock(anmalloc_lock_t * lock) {
  basic_lock_unlock(lock);
}

