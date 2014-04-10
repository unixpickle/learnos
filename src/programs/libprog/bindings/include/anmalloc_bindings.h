#include <stdint.h>
#include <stddef.h>

typedef struct anmalloc_waiting_t anmalloc_waiting_t;

struct anmalloc_waiting_t {
  uint64_t threadIndex;
  anmalloc_waiting_t * next;
} __attribute__((packed));

typedef struct {
  anmalloc_waiting_t * waiting;
  uint64_t isHeld;
  uint64_t baseLock;
} __attribute__((packed)) anmalloc_lock_t;

#define ANMALLOC_LOCK_INITIALIZER {NULL, 0, 0}

void * anmalloc_sbrk(intptr_t incr);
int anmalloc_brk(const void * addr);

void anmalloc_lock(anmalloc_lock_t * lock);
void anmalloc_unlock(anmalloc_lock_t * lock);
