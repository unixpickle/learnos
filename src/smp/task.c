#include <anlock.h>
#include <kernpage.h>
#include <shared/addresses.h>
#include <libkern_base.h>
#include "task.h"

static uint64_t _next_pid();
static bool _create_4mb_identity(uint64_t * pml4);
static void _deallocate_table(uint64_t * table, int depth);

void * task_idxset_alloc_safe() {
  task_critical_start();
  kernpage_lock();
  page_t page = kernpage_alloc_virtual();
  kernpage_unlock();
  task_critical_stop();
  return (void *)(page << 12);
}

void task_idxset_free_safe(void * ptr) {
  task_critical_start();
  kernpage_lock();
  kernpage_free_virtual(((uint64_t)ptr) >> 12);
  kernpage_unlock();
  task_critical_stop();
}

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

void task_critical_start() {
  disable_interrupts();
}

void task_critical_stop() {
  enable_interrupts();
}

void tasks_initialize() {
  tasks_root_t * root = (tasks_root_t *)TASK_LIST_PTR;
  anlock_initialize(&root->lock);
  anlock_initialize(&root->pidsLock);
  root->firstTask = NULL;
  root->nextTask = NULL;
  anidxset_initialize(&root->pids, task_idxset_alloc, task_idxset_free);
}

task_t * task_create() {
  kernpage_lock();
  page_t taskPage = kernpage_alloc_virtual();
  page_t pml4 = kernpage_alloc_virtual();
  kernpage_unlock();

  if (!taskPage || !pml4) {
    kernpage_lock();
    if (taskPage) kernpage_free_virtual(taskPage);
    if (pml4) kernpage_free_virtual(pml4);
    kernpage_unlock();
    return NULL;
  }

  task_t * task = (task_t *)(taskPage << 12);
  ref_initialize(task, (void (*)(void *))task_dealloc);
  task->pml4 = kernpage_calculate_physical(pml4);
  task->pid = _next_pid();
  task->uid = 0;
  task->isActive = false;
  task->nextTask = NULL;
  task->firstThread = NULL;
  task->nextThread = NULL;
  task->firstSocket = NULL;
  
  anlock_initialize(&task->nextTaskLock);
  anlock_initialize(&task->pml4Lock);
  anlock_initialize(&task->threadsLock);
  anlock_initialize(&task->threadStacksLock);
  anlock_initialize(&task->firstSocketLock);
  anlock_initialize(&task->socketDescsLock);

  // allocate descriptor number tables
  if (!anidxset_initialize(&task->socketDescs,
                           task_idxset_alloc,
                           task_idxset_free)) {
    kernpage_lock();
    kernpage_free_virtual(taskPage);
    kernpage_free_virtual(pml4);
    kernpage_unlock();
    return NULL;
  }
  if (!anidxset_initialize(&task->threadStacks,
                           task_idxset_alloc,
                           task_idxset_free)) {
    kernpage_lock();
    kernpage_free_virtual(taskPage);
    kernpage_free_virtual(pml4);
    kernpage_unlock();
    anidxset_free(&task->socketDescs);
    return NULL;
  }

  // allocate initial page tables
  if (!_create_4mb_identity((uint64_t *)(pml4 << 12))) {
    kernpage_lock();
    kernpage_free_virtual(taskPage);
    kernpage_free_virtual(pml4);
    kernpage_unlock();
    anidxset_free(&task->socketDescs);
    anidxset_free(&task->threadStacks);
    return NULL;
  }

  return task;
}

void task_dealloc(task_t * task) {
  // deallocate all memory mapping structure
  uint64_t vPml4 = kernpage_calculate_virtual(task->pml4);
  uint64_t * pml4 = (uint64_t *)(vPml4 << 12);
  _deallocate_table(pml4, 0);
  kernpage_lock();
  kernpage_free_virtual(vPml4);
  kernpage_unlock();

  // the cleanup workers should have already free'd these appropriately, but
  // just in case, we do it here.
  anidxset_free(&task->socketDescs);
  anidxset_free(&task->threadStacks);
  
  // relinquish the PID
  tasks_root_t * root = (tasks_root_t *)TASK_LIST_PTR;
  anlock_lock(&root->pidsLock);
  anidxset_put(&root->pids, task->pid);
  anlock_unlock(&root->pidsLock);
}

static uint64_t _next_pid() {
  tasks_root_t * root = (tasks_root_t *)TASK_LIST_PTR;
  anlock_lock(&root->pidsLock);
  uint64_t pid = anidxset_get(&root->pids);
  anlock_unlock(&root->pidsLock);
  return pid;
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

static void _deallocate_table(uint64_t * table, int depth) {
  if (depth == 3) return;

  int i;
  for (i = 0; i < 0x200; i++) {
    if (!(table[i] & 1)) continue;
    page_t tablePage = table[i] >> 12;
    page_t vPage = kernpage_calculate_virtual(tablePage);
    uint64_t * subtable = (uint64_t *)(vPage << 12);
    _deallocate_table(subtable, depth + 1);
    kernpage_lock();
    kernpage_free_virtual(vPage);
    kernpage_unlock();
  }
}

