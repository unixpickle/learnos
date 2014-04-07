#include <anscheduler/interrupts.h>
#include <anscheduler/loop.h>
#include <anscheduler/task.h>
#include <anscheduler/functions.h>

static thread_t * interruptThread __attribute__((aligned(8))) = NULL;
static uint64_t intdLock __attribute__((aligned(8))) = 0;
static uint32_t irqMask __attribute__((aligned(8))) = 0;

void anscheduler_irq(uint8_t irqNumber) {
  // while our threadLock is held, the interruptThread cannot be deallocated
  // so we don't have to worry about it going away.
  anscheduler_intd_lock();  
  if (!interruptThread) {
    anscheduler_intd_unlock();
    return;
  }
  task_t * task = interruptThread->task;
  if (!anscheduler_task_reference(task)) {
    anscheduler_intd_unlock();
    return;
  }
  
  anscheduler_or_32(&irqMask, (1 << irqNumber));
  bool result = __sync_fetch_and_and(&interruptThread->isPolling, 0);
  anscheduler_intd_unlock();
  
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
  anscheduler_intd_lock();
  thread_t * result = interruptThread;
  anscheduler_intd_unlock();
  return result;
}

void anscheduler_intd_set(thread_t * thread) {
  anscheduler_intd_lock();
  interruptThread = thread;
  anscheduler_intd_unlock();
}

void anscheduler_intd_cmpnull(thread_t * thread) {
  anscheduler_intd_lock();
  if (interruptThread == thread) {
    interruptThread = NULL;
  }
  anscheduler_intd_unlock();
}

uint32_t anscheduler_intd_read() {
  anscheduler_intd_lock();
  uint32_t ret = irqMask;
  irqMask = 0;
  anscheduler_intd_unlock();
  return ret;
}

void anscheduler_intd_lock() {
  anscheduler_lock(&intdLock);
}

void anscheduler_intd_unlock() {
  anscheduler_unlock(&intdLock);
}

bool anscheduler_intd_waiting() {
  return irqMask != 0;
}
