#include "scheduler.h"
#include "cpu.h"
#include "context.h"

#include <anlock.h>
#include <libkern_base.h>


static thread_t * firstThread = NULL;
static thread_t * lastThread = NULL;
static uint64_t queueLock = 0;
static uint64_t systemTimestamp = 0;

void scheduler_handle_timer() {
  lapic_send_eoi();
  scheduler_stop_current();
  scheduler_task_loop();
}

void scheduler_flush_timer() {
  cpu_t * cpu = cpu_current();
  uint64_t passed = cpu->lastTimeout - lapic_get_register(LAPIC_REG_TMRCURRCNT);
  // do an atomic addition
  __asm__("lock add %1, (%0)"
          :
          : "b" (&systemTimestamp), "a" (passed)
          : "memory");
  lapic_set_register(LAPIC_REG_LVT_TMR, 0x10000); // disable timer
}

uint64_t scheduler_get_timestamp() {
  uint64_t timestamp;
  __asm__("lock mov (%1), %0",
          : "=a" (timestamp)
          : "b" (&systemTimestamp));
  return timestamp;
}

void scheduler_stop_current() {
  cpu_t * cpu = cpu_current();
  if (!cpu->thread) return;
  thread_t * thread = cpu->thread;
  task_t * task = cpu->task;
  cpu->thread = NULL;
  cpu->task = NULL;

  anlock_lock(&thread->stateLock);
  uint64_t state = (thread->state ^= 1);
  if (!state) {
    // push it back to the queue
    task_queue_lock();
    task_queue_push(thread);
    task_queue_unlock();
  }
  anlock_unlock(&thread->stateLock);
}

void scheduler_run_next() {
  // TODO: this function should also keep track of how much time passes while
  // trying to find the next task in the queue to run.

  uint64_t timestamp = scheduler_get_timestamp();

  // minimum of 32 ticks/second
  uint64_t nextTimestamp = timestamp + (lapic_get_bus_speed() >> 5);

  task_queue_lock();
  thread_t * thread = task_queue_pop();
  thread_t * firstThread = thread;

  if (!thread) {
    task_queue_unlock();
    return;
  }

  do {
    if (thread->nextTimestamp <= timestamp) {
      break;
    }
    if (thread->nextTimestamp < nextTimestamp) {
      nextTimestamp = thread->nextTimestamp;
    }
    task_queue_push(thread);
    thread = task_queue_pop();
  } while (thread != firstThread);
  task_queue_unlock();

  if (thread->nextTimestamp > timestamp) return;

  cpu_t * cpu = cpu_current();
  cpu->lastTimeout = nextTimestamp - timestamp;
  lapic_timer_set(0x20, 0xb, nextTimestamp - timestamp);

  // do logic here
  anlock_lock(&thread->stateLock);
  thread->state |= 1;
  anlock_unlock(&thread->stateLock);
  cpu->thread = thread;
  cpu->task = thread->task;

  task_switch(thread->task, thread);
}

void scheduler_task_loop() {
  scheduler_flush_timer();
  scheduler_run_next();

  lapic_timeout_set(0x20, 0xb, lapic_get_bus_speed() >> 7);
  enable_interrupts();
  while (1) halt();
}

/**************
 * Task queue *
 *************/

void task_queue_lock() {
  anlock_lock(&queueLock);
}

void task_queue_unlock() {
  anlock_unlock(&queueLock);
}

void task_queue_push(thread_t * item) {
  item->queueNext = NULL;
  item->queueLast = lastThread;
  if (lastThread) {
    lastThread->queueNext = item;
  } else {
    firstThread = item;
  }
  lastThread = item;
}

void task_queue_push_first(thread_t * item) {
  item->queueLast = NULL;
  item->queueNext = firstThread;
  if (firstThread) firstThread->queueLast = item;
  firstThread = item;
}

thread_t * task_queue_pop() {
  thread_t * first = firstThread;
  if (!first) return NULL;

  firstThread = first->queueNext;
  if (firstThread) {
    firstThread->queueLast = NULL;
  } else {
    lastThread = NULL;
  }
  return first;
}

void task_queue_remove(thread_t * item) {
  if (item->queueLast) {
    item->queueLast->next = item->queueNext;
  } else {
    firstThread = item->next;
  }
  if (item->next) {
    item->next->last = item->last;
  } else {
    lastThread = item->last;
  }
}

