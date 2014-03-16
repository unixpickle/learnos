#include "types.h"
#include "util.h"
#include <anlock.h>

static uint64_t tasksLock = 0;
static task_t * firstTask = NULL;

static uint64_t pidsLock = 0;
static anidxset_root_t pids;

/*************
 * PIDs list *
 *************/

void pids_initialize() {
  anidxset_initialize(&pids, task_idxset_alloc, task_idxset_free);
}

uint64_t pids_next() {
  anlock_lock(&pidsLock);
  uint64_t i = anidxset_get(&pids);
  anlock_unlock(&pidsLock);
  return i;
}

void pids_release(uint64_t pid) {
  anlock_lock(&pidsLock);
  anidxset_put(&pids, pid);
  anlock_unlock(&pidsLock);
}

/**************
 * Tasks list *
 **************/

void tasks_lock() {
  anlock_lock(&tasksLock);
}

void tasks_add(task_t * task) {
  task->lastTask = NULL;
  task->nextTask = firstTask;
  if (firstTask) firstTask->lastTask = task;
  firstTask = task;
}

void tasks_remove(task_t * task) {
  if (task->lastTask) {
    task->lastTask->nextTask = task->nextTask;
  } else {
    firstTask = task->nextTask;
  }

  if (task->nextTask) {
    task->nextTask->lastTask = task->lastTask;
  }
}

task_t * tasks_find(uint64_t pid) {
  task_t * cur = firstTask;
  while (cur) {
    if (cur->pid == pid) return cur;
    cur = cur->nextTask;
  }
  return NULL;
}

void tasks_iterate(uint64_t value, void (* iterator)(uint64_t v, task_t * t)) {
  task_t * cur = firstTask;
  while (cur) {
    iterator(value, cur);
    cur = cur->nextTask;
  }
}

void tasks_unlock() {
  anlock_unlock(&tasksLock);
}

