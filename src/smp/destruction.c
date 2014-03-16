#include "destruction.h"
#include <kernpage.h>

void thread_dealloc(thread_t * thread) {
  kernpage_lock();
  kernpage_free_virtual(((page_t)thread) >> 12);
  kernpage_unlock();
}

void task_dealloc(task_t * task) {
  kernpage_lock();
  kernpage_free_virtual(((page_t)task) >> 12);
  kernpage_unlock();
}

