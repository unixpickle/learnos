#include "pidmap.h"
#include "util.h"
#include <anscheduler/functions.h>
#include <anscheduler/task.h>

static uint64_t pmLock __attribute__((aligned(8))) = 0;
static task_t * pidHashmap[0x100] __attribute__((aligned(8)));

static uint64_t ppLock __attribute__((aligned(8))) = 0;
static anidxset_root_t pidPool __attribute__((aligned(8)));
static bool ppInitialized __attribute__((aligned(8))) = 0;

static uint8_t _hash_pid(uint64_t pid);

uint64_t anscheduler_pidmap_alloc_pid() {
  anscheduler_lock(&ppLock);
  if (!ppInitialized) {
    if (!anscheduler_idxset_init(&pidPool)) {
      anscheduler_abort("failed to initialize PID pool");
    }
    ppInitialized = true;
  }
  uint64_t result = anidxset_get(&pidPool);
  anscheduler_unlock(&ppLock);
  return result;
}

void anscheduler_pidmap_free_pid(uint64_t pid) {
  anscheduler_lock(&ppLock);
  if (!ppInitialized) {
    anscheduler_abort("cannot free PID when not initialized");
  }
  anidxset_put(&pidPool, pid);
  anscheduler_unlock(&ppLock);
}

void anscheduler_pidmap_set(task_t * task) {
  anscheduler_lock(&pmLock);
  uint8_t hash = _hash_pid(task->pid);
  if (!pidHashmap[hash]) {
    pidHashmap[hash] = task;
    task->next = (task->last = NULL);
  } else {
    task->next = pidHashmap[hash];
    task->last = NULL;
    pidHashmap[hash]->last = task;
    pidHashmap[hash] = task;
  }
  anscheduler_unlock(&pmLock);
}

void anscheduler_pidmap_unset(task_t * task) {
  anscheduler_lock(&pmLock);
  if (!task->last) {
    uint8_t hash = _hash_pid(task->pid);
    pidHashmap[hash] = task->next;
  } else {
    task->last->next = task->next;
  }
  if (task->next) {
    task->next->last = task->last;
  }
  task->next = NULL;
  task->last = NULL;
  anscheduler_unlock(&pmLock);
}

task_t * anscheduler_pidmap_get(uint64_t pid) {
  anscheduler_lock(&pmLock);
  uint8_t hash = _hash_pid(pid);
  task_t * task = pidHashmap[hash];
  while (task) {
    if (task->pid == pid) {
      bool result = anscheduler_task_reference(task);
      anscheduler_unlock(&pmLock);
      return result ? task : NULL;
    }
    task = task->next;
  }
  anscheduler_unlock(&pmLock);
  return NULL;
}

static uint8_t _hash_pid(uint64_t pid) {
  return (uint8_t)(pid & 0xff);
}
