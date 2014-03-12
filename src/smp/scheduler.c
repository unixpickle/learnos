#include "scheduler.h"
#include "context.h"
#include "thread.h"
#include <anlock.h>
#include <shared/addresses.h>
#include "cpu_list.h"

void scheduler_switch_task(task_t * task, thread_t * thread) {
  cpu_info * info = cpu_get_current();
  anlock_lock(&info->lock);
  if (info->currentThread) {
    anlock_lock(&thread->statusLock);
    thread->status ^= 1;
    anlock_unlock(&thread->statusLock);
  }
  info->currentTask = task;
  info->currentThread = thread;
  thread_configure_tss(thread, info->tss);

  // get the TSS and set it if it's wrong
  // TODO: see if we need to reload the TSS
  uint16_t currentTss;
  __asm__ ("str %0" : "=r" (currentTss));
  if (currentTss != info->tssSelector) {
    __asm__ ("ltr %0" : : "r" (info->tssSelector));
  }

  anlock_unlock(&info->lock);
  task_switch(task, thread); // changes our execution context
}

void scheduler_run_next() {
  cpu_info * info = cpu_get_current();
  anlock_lock(&info->lock);
  if (info->currentThread) {
    anlock_lock(&info->currentThread->statusLock);
    info->currentThread->status ^= 1;
    anlock_unlock(&info->currentThread->statusLock);
  }
  info->currentTask = NULL;
  info->currentThread = NULL;
  anlock_unlock(&info->lock);

  task_t * task;
  thread_t * thread;
  if (task_get_next_job(&task, &thread)) {
    scheduler_switch_task(task, thread);
  }
}

bool scheduler_generate_task(void * code, uint64_t len) {
  task_t * task = task_create();
  if (!task) return false;

  thread_t * thread = thread_create_first(task, code, len);
  if (!thread) {
    return false;
  }

  task->nextThread = thread;
  task->firstThread = thread;
  task_list_add(task);
  return true;
}

void scheduler_handle_interrupt(uint64_t irqMask) {
  tasks_root_t * root = (tasks_root_t *)TASK_LIST_PTR;
  anlock_lock(&root->lock);
  task_t * task = root->firstTask;
  while (task) {
    anlock_lock(&task->threadsLock);
    thread_t * thread = task->firstThread;
    while (thread) {
      __asm__ ("lock orq %0, (%1)" :
               : "a" (irqMask), "b" (&thread->interruptMask)
               : "memory");
      anlock_lock(&thread->statusLock);
      if (thread->status & 0x2) {
        thread->status ^= 2;
        uint64_t newRax = 0;
        __asm__ ("lock xchgq %0, (%1)"
                 : "=a" (newRax)
                 : "r" (&thread->interruptMask)
                 : "memory");
        thread->state.rax = newRax;
      }
      anlock_unlock(&thread->statusLock);
      thread = thread->nextThread;
    }
    anlock_unlock(&task->threadsLock);
    task = task->nextTask;
  }
  anlock_unlock(&root->lock);
}

