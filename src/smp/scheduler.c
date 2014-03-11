#include "scheduler.h"
#include "context.h"
#include "thread.h"
#include <anlock.h>
#include "cpu_list.h"

void scheduler_switch_task(task_t * task, thread_t * thread) {
  cpu_info * info = cpu_get_current();
  anlock_lock(&info->lock);
  if (info->currentThread) {
    // TODO: figure out a better way than with sync
    __sync_fetch_and_and(&info->currentThread->runState, 0b11111110);
  }
  ref_release(info->currentTask);
  ref_release(info->currentThread);
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
  // TODO: see if we need to use __sync
  if (info->currentThread) {
    __sync_fetch_and_and(&info->currentThread->runState, 0b11111110);
  }
  task_t * oldTask = info->currentTask;
  ref_release(info->currentTask);
  ref_release(info->currentThread);
  info->currentTask = NULL;
  info->currentThread = NULL;
  anlock_unlock(&info->lock);

  if (oldTask) return;

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
    ref_release(task);
    return false;
  }

  task->nextThread = thread;
  task->firstThread = ref_retain(thread);
  task_list_add(task);
  ref_release(task);
  return true;
}

