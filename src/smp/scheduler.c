#include "scheduler.h"
#include "cpu.h"
#include "lifecycle.h"
#include "context.h"
#include "timer.h"

#include <interrupts/lapic.h>
#include <anlock.h>
#include <libkern_base.h>


static thread_t * firstThread = NULL;
static thread_t * lastThread = NULL;
static uint64_t queueLock = 0;

void scheduler_handle_timer() {
  timer_send_eoi();
  scheduler_stop_current();
  scheduler_task_loop();
}

void scheduler_stop_current() {
  cpu_t * cpu = cpu_current();
  if (!cpu->thread) return;
  thread_t * thread = cpu->thread;
  cpu->thread = NULL;
  cpu->task = NULL;

  anlock_lock(&thread->statusLock);
  uint64_t state = (thread->status ^= 1);
  anlock_unlock(&thread->statusLock);
  if (!state) {
    // push it back to the queue
    task_queue_lock();
    task_queue_push(thread);
    task_queue_unlock();
  }
}

void scheduler_run_next() {
  timer_set_far_timeout(); // start timing this little excursion
  uint64_t timestamp = timer_get_time();

  // the absolute maximum amount of time we can wait
  uint64_t nextTimestamp = timestamp + (lapic_get_bus_speed() >> 7);

  task_queue_lock();
  thread_t * thread = task_queue_pop();
  thread_t * firstThread = thread;

  if (!thread) {
    task_queue_unlock();
    return;
  }

  bool foundOne = false;
  do {
    if (thread->nextTimestamp <= timestamp) {
      foundOne = true;
      break;
    }
    if (thread->nextTimestamp < nextTimestamp) {
      nextTimestamp = thread->nextTimestamp;
    }
    task_queue_push(thread);
    thread = task_queue_pop();
  } while (thread != firstThread);

  if (!foundOne) {
    task_queue_push(thread);
    task_queue_unlock();
    return;
  }

  anlock_lock(&thread->statusLock);
  thread->status |= 1;
  anlock_unlock(&thread->statusLock);

  task_queue_unlock();

  cpu_t * cpu = cpu_current();
  timer_set_timeout(nextTimestamp - timestamp);

  // do logic here
  cpu->thread = thread;
  cpu->task = thread->task;

  thread_configure_tss(thread, cpu->tss);

  // TODO: check if we need to change the TSS
  task_switch(thread->task, thread);
}

void scheduler_task_loop() {
  scheduler_run_next();

  timer_set_timeout(lapic_get_bus_speed() >> 7);
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
  if (item->queueNext || item->queueLast) return;
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
  if (item->queueNext || item->queueLast) return;
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

