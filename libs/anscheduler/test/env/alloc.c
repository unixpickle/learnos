#include "alloc.h"
#include "threading.h"
#include <assert.h>
#include <stdlib.h>

static uint64_t allocedPieces __attribute__((aligned(8))) = 0;

void * anscheduler_alloc(uint64_t size) {
  assert(antest_get_current_cpu_info()->isLocked);
  assert((((uint64_t)&allocedPieces) & 0x7) == 0);
  __asm__ __volatile__("lock incq (%0)" : : "r" (&allocedPieces));
  
  if (size > 0x1000) return NULL;
  
  void * buf;
  posix_memalign(&buf, 0x1000, size);
  return buf;
}

void anscheduler_free(void * buffer) {
  assert(antest_get_current_cpu_info()->isLocked);
  __asm__ __volatile__("lock decq (%0)" : : "r" (&allocedPieces));
  free(buffer);
}

uint64_t antest_pages_alloced() {
  return allocedPieces;
}
