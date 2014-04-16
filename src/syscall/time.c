#include "time.h"
#include "vm.h"
#include <anscheduler/thread.h>
#include <anscheduler/task.h>
#include <anscheduler/loop.h>
#include <anscheduler/functions.h>

static void _sleep_method(thread_t * th, uint64_t usec);

uint64_t syscall_get_time() {
  anscheduler_cpu_lock();
  uint64_t ts = anscheduler_get_time();
  // provide a decent timestamp, but not perfect
  uint64_t res = 1000 * ts / (anscheduler_second_length() / 1000);
  anscheduler_cpu_unlock();
  return res;
}

void syscall_sleep(uint64_t usec) {
  anscheduler_cpu_lock();
  thread_t * thread = anscheduler_cpu_get_thread();
  anscheduler_lock(&thread->state.sleepLock);
  _sleep_method(thread, usec);
  anscheduler_cpu_unlock();
}

void syscall_unsleep(uint64_t thread) {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  anscheduler_lock(&task->threadsLock);
  thread_t * target = task->firstThread;
  while (target) {
    if (target->stack == thread) break;
    target = target->next;
  }
  anscheduler_unlock(&task->threadsLock);
  if (target) {
    anscheduler_lock(&target->state.sleepLock);
    target->nextTimestamp = 0;
    anscheduler_unlock(&target->state.sleepLock);
  }
  anscheduler_cpu_unlock();
}

uint64_t syscall_atomic_sleep(uint64_t usec, uint64_t condPtr) {
  anscheduler_cpu_lock();
  if (condPtr & 8) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_MEMORY);
  }
  thread_t * thread = anscheduler_cpu_get_thread();
  volatile uint64_t * valPtr;
  if (!task_get_virtual((void *)condPtr, (void **)&valPtr)) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_MEMORY);
  }

  // while sleep lock is held, check if we should really sleep at all
  anscheduler_lock(&thread->state.sleepLock);
  uint64_t val = __sync_fetch_and_and(valPtr, 0);
  if (val) {
    _sleep_method(thread, usec);
  } else {
    anscheduler_unlock(&thread->state.sleepLock);
  }
  anscheduler_cpu_unlock();
  return val ? 1 : 0;
}

static void _sleep_method(thread_t * th, uint64_t usec) {
  uint64_t units = (anscheduler_second_length() * usec) / 1000000L;
  uint64_t now = anscheduler_get_time();
  uint64_t destTime = now + units;
  if (destTime < now) destTime = 0xffffffffffffffffL;
  th->nextTimestamp = destTime;
  anscheduler_unlock(&th->state.sleepLock);
  anscheduler_loop_save_and_resign();
}

