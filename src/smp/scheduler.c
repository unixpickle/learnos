#include "scheduler.h"
#include "context.h"
#include <anlock.h>
#include "cpu_list.h"

void scheduler_switch_task(task_t * task, thread_t * thread) {
  cpu_info * info = cpu_get_current();
  anlock_lock(&info->lock);
  __sync_fetch_and_and(&info->currentThread->isRunning, 0);
  ref_release(info->currentTask);
  ref_release(info->currentThread);
  info->currentTask = task;
  info->currentThread = thread;
  anlock_unlock(&info->lock);

  // TODO: here, we must configure the TSS to reflect the state of `thread`

  task_switch(task, thread); // changes our execution context
}

void scheduler_run_next() {
  cpu_info * info = cpu_get_current();
  anlock_lock(&info->lock);
  // TODO: see if we need a __sync macro here
  __sync_fetch_and_and(&info->currentThread->isRunning, 0);
  ref_release(info->currentTask);
  ref_release(info->currentThread);
  info->currentTask = NULL;
  info->currentThread = NULL;
  anlock_unlock(&info->lock);

  task_t * task;
  thread_t * thread;
  if (task_get_next_job(&task, &thread)) {
    scheduler_switch_task(task, thread);
  }
}

