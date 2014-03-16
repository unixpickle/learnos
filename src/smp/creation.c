#include "types.h"
#include "destruction.h"
#include "util.h"
#include "tasks.h"
#include <kernpage.h>
#include <libkern_base.h>

static bool _create_4mb_identity(uint64_t * pml4);

task_t * task_alloc() {
  kernpage_lock();
  page_t taskPage = kernpage_alloc_virtual();
  page_t pml4 = kernpage_alloc_virtual();
  kernpage_unlock();

  if (!taskPage || !pml4) {
    kernpage_lock();
    if (pml4) kernpage_free_virtual(pml4);
    if (taskPage) kernpage_free_virtual(taskPage);
    kernpage_unlock();
    return NULL;
  }

  zero_page(pml4);
  zero_page(taskPage);

  task_t * task = (task_t *)(taskPage << 12);
  task->pml4 = kernpage_calculate_physical(pml4);
  task->pid = pids_next();

  if (!anidxset_initialize(&task->descriptors,
                           task_idxset_alloc,
                           task_idxset_free)) {
    kernpage_lock();
    kernpage_free_virtual(taskPage);
    kernpage_free_virtual(pml4);
    kernpage_unlock();
    return NULL;
  }

  if (!anidxset_initialize(&task->stacks,
                           task_idxset_alloc,
                           task_idxset_free)) {
    kernpage_lock();
    kernpage_free_virtual(taskPage);
    kernpage_free_virtual(pml4);
    kernpage_unlock();
    anidxset_free(&task->descriptors);
    return NULL;
  }

  if (!_create_4mb_identity((void *)(pml4 << 12))) {
    kernpage_lock();
    kernpage_free_virtual(taskPage);
    kernpage_free_virtual(pml4);
    kernpage_unlock();
    anidxset_free(&task->descriptors);
    anidxset_free(&task->stacks);
    return NULL;
  }

  return task;
}

thread_t * thread_alloc(task_t * task) {
  // TODO: create thread here
  return NULL;
}

void thread_setup_first(thread_t * thread, void * program, uint64_t len) {
  
}

void thread_setup(thread_t * thread, void * rip) {
}

static bool _create_4mb_identity(uint64_t * pml4) {
  kernpage_lock();
  page_t pdpt = kernpage_alloc_virtual();
  page_t pdt = kernpage_alloc_virtual();
  page_t pt1 = kernpage_alloc_virtual();
  page_t pt2 = kernpage_alloc_virtual();
  kernpage_unlock();
  if (!pdpt || !pdt || !pt1 || !pt2) {
    kernpage_lock();
    if (!pt2) kernpage_free_virtual(pt2);
    if (!pt1) kernpage_free_virtual(pt1);
    if (!pdt) kernpage_free_virtual(pdt);
    if (!pdpt) kernpage_free_virtual(pdpt);
    kernpage_unlock();
    return false;
  }
  zero_page(pdpt);
  zero_page(pdt);
  pml4[0] = (kernpage_calculate_physical(pdpt) << 12) | 3;
  uint64_t * table = (uint64_t *)(pdpt << 12);
  table[0] = (kernpage_calculate_physical(pdt) << 12) | 3;
  table = (uint64_t *)(pdt << 12);
  table[0] = (kernpage_calculate_physical(pt1) << 12) | 3;
  table[1] = (kernpage_calculate_physical(pt2) << 12) | 3;
  table = (uint64_t *)(pt1 << 12);
  uint64_t i;
  for (i = 0; i < 0x200; i++) {
    table[i] = (i << 12) | 3;
  }
  table = (uint64_t *)(pt2 << 12);
  for (i = 0; i < 0x200; i++) {
    table[i] = ((i + 0x200) << 12) | 3;
  }
  return true;
}

