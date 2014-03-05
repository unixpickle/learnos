#include <anlock.h>
#include "task.h"

static uint64_t _next_pid();

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
  task->pml4 = kernpage_calculate_physical(pml4);
  task->pid = _next_pid();
  task->uid = 0;
  task->isActive = false;
  
  anlock_initialize(&task->pml4Lock);
  anlock_initialize(&task->threadsLock);
  anlock_initialize(&task->threadStacksLock);
  anlock_initialize(&task->firstSocketLock);
  anlock_initialize(&task->socketDescsLock);
  if (!anidxset_initialize(&task->socketDescs,
                           task_idxset_alloc,
                           task_idxset_free)) {
    kernpage_lock();
    kernpage_free_virtual(taskPage);
    kernpage_free_virtual(pml4);
    kernpage_unlock();
  }
  anidxset_initialize(&task->threadStacks, task_idxset_alloc, task_idxset_free);
}

void task_dealloc(task_t * task) {
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

