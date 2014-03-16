#include <kernpage.h>

void * task_idxset_alloc() {
  kernpage_lock();
  page_t page = kernpage_alloc_virtual();
  kernpage_unlock();
  return (void *)(page << 12);
}

void task_idxset_free(void * ptr) {
  kernpage_lock();
  kernpage_free_virtual(((uint64_t)ptr) >> 12);
  kernpage_unlock();
}

