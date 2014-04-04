#include <anscheduler/interrupts.h>
#include <anscheduler/loop.h>
#include <anscheduler/task.h>
#include <anscheduler/functions.h>

static thread_t * interruptThread __attribute__((aligned(8))) = NULL;
static uint64_t threadLock __attribute__((aligned(8))) = 0;
static uint32_t irqMask __attribute__((aligned(8))) = 0;

void anscheduler_irq(uint8_t irqNumber) {
  // while our threadLock is held, the interruptThread cannot be deallocated
  // so we don't have to worry about it going away.
  anscheduler_lock(&threadLock);
  if (!interruptThread) {
    anscheduler_unlock(&threadLock);
    return;
  }
  task_t * task = interruptThread->task;
  if (!anscheduler_task_reference(task)) {
    anscheduler_unlock(&threadLock);
    return;
  }
  
  anscheduler_or_32(&irqMask, (1 << irqNumber));
  bool result = __sync_fetch_and_and(&interruptThread->isPolling, 0);
  anscheduler_unlock(&threadLock);
  
  if (result) { // the thread was polling!
    // we know our reference to interruptThread is still valid, because if
    // a thread is polling, it cannot be killed unless its task is killed, and
    // we hold a reference to its task.
    anscheduler_loop_switch(task, interruptThread);
  } else {
    anscheduler_task_dereference(task);
  }
}

thread_t * anscheduler_intd_get() {
  anscheduler_lock(&threadLock);
  thread_t * result = interruptThread;
  anscheduler_unlock(&threadLock);
  return result;
}

void anscheduler_intd_set(thread_t * thread) {
  anscheduler_lock(&threadLock);
  interruptThread = thread;
  anscheduler_unlock(&threadLock);
}

void anscheduler_intd_cmpnull(thread_t * thread) {
  anscheduler_lock(&threadLock);
  if (interruptThread == thread) {
    interruptThread = NULL;
  }
  anscheduler_unlock(&threadLock);
}

uint32_t anscheduler_intd_read() {
  return __sync_fetch_and_and(&irqMask, 0);
}
