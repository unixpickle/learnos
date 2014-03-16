#include "scheduler.h"
#include "cpu.h"
#include "lifecycle.h"
#include "context.h"

#include <interrupts/lapic.h>
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
  if (!cpu->lastTimeout) return;
  // TODO: here, check if a timer interrupt is in progress. If so, do nothing.
  uint64_t passed = cpu->lastTimeout - lapic_get_register(LAPIC_REG_TMRCURRCNT);
  // do an atomic addition
  __asm__("lock add %1, (%0)"
          :
          : "b" (&systemTimestamp), "a" (passed)
          : "memory");
  lapic_set_register(LAPIC_REG_LVT_TMR, 0x10000); // disable timer
}

uint64_t scheduler_get_timestamp() {
  // TODO: this is prettyl lowsy, but it'll do
  return __sync_fetch_and_or(&systemTimestamp, 0);
}

void scheduler_stop_current() {
  cpu_t * cpu = cpu_current();
  if (!cpu->thread) return;
  thread_t * thread = cpu->thread;
  cpu->thread = NULL;
  cpu->task = NULL;

  anlock_lock(&thread->statusLock);
  uint64_t state = (thread->status ^= 1);
  if (!state) {
    // push it back to the queue
    task_queue_lock();
    task_queue_push(thread);
    task_queue_unlock();
  }
  anlock_unlock(&thread->statusLock);
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
  lapic_timer_set(0x20, 0, (nextTimestamp - timestamp) >> 1);

  // do logic here
  anlock_lock(&thread->statusLock);
  thread->status |= 1;
  anlock_unlock(&thread->statusLock);
  cpu->thread = thread;
  cpu->task = thread->task;

  thread_configure_tss(thread, cpu->tss);

  // TODO: see if we need to reload the TSS
  task_switch(thread->task, thread);
}

void scheduler_task_loop() {
  scheduler_flush_timer();
  scheduler_run_next();

  lapic_timer_set(0x20, 0, lapic_get_bus_speed() >> 8);
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
  first->queueNext = (first->queueLast = NULL);
  return first;
}

void task_queue_remove(thread_t * item) {
  if (!item->queueLast && !item->queueNext && item != firstThread) {
    return;
  }
  if (item->queueLast) {
    item->queueLast->queueNext = item->queueNext;
  } else {
    firstThread = item->queueNext;
  }
  if (item->queueNext) {
    item->queueNext->queueLast = item->queueLast;
  } else {
    lastThread = item->queueLast;
  }
  item->queueNext = (item->queueLast = NULL);
}

