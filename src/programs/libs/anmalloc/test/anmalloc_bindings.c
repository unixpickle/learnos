#include <anmalloc_bindings.h>
#include <stdlib.h>
#include <assert.h>

static void * buffer = NULL;
static intptr_t used = 0;
static intptr_t allocated = 0;

void * anmalloc_sbrk(intptr_t incr) {
  if (!allocated) {
    allocated = 0x1000000;
    int res = posix_memalign(&buffer, (size_t)allocated, (size_t)allocated);
    assert(res == 0);
  }
  
  void * result = buffer + used;
  
  if (incr < 0 && used + incr < 0) {
    return (void *)-1;
  } else if (used + incr > allocated) {
    return (void *)-1;
  }
  
  buffer += incr;
  used += incr;
  return result;
}

int anmalloc_brk(const void * addr) {
  intptr_t difference = ((intptr_t)addr) - ((intptr_t)buffer);
  void * buf = anmalloc_sbrk(difference);
  if (buf == (void *)-1) return -1;
  return 0;
}

void anmalloc_lock(anmalloc_lock_t * lock) {
  pthread_mutex_lock(lock);
}

void anmalloc_unlock(anmalloc_lock_t * lock) {
  pthread_mutex_unlock(lock);
}

uint64_t __anmalloc_brk_size() {
  return used;
}
