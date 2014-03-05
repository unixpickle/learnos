#include <anlock.h>
#include "task.h"

void * task_idxset_alloc() {
  kernpage_lock();
  page_t page = kernpage_alloc_virtual();
  kernpage_unlock();
  return (page << 12);
}

void task_idxset_free(void * ptr) {
  kernpage_lock();
  kernpage_free_virtual(((uint64_t)ptr) >> 12);
  kernpage_unlock();
}

void task_critical_start() {
  disable_interrupts();
}

void task_critical_stop() {
  enable_interrupts();
}

task_t * task_create() {
  task_critical_start();
  kernpage_lock();
  page_t taskPage = kernpage_alloc_virtual();
  page_t pml4 = kernpage_alloc_virtual();
  kernpage_unlock();
  
}

void task_dealloc(task_t * task) {
}

