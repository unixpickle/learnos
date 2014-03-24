/**
 * For testing purposes, I need a way to emulate multiple CPU cores. In this
 * system, each pthread is a CPU core.
 */

#include <anscheduler/types.h>

typedef struct {
  task_t * task;
  thread_t * thread;
  bool isLocked;
  uint64_t nextInterrupt;
  void * cpuStack;
} cpu_info;

cpu_info * antest_get_current_cpu_info();
void antest_launch_thread(void * arg, void (* method)(void *));

void anscheduler_cpu_lock();
void anscheduler_cpu_unlock();
task_t * anscheduler_cpu_get_task();
thread_t * anscheduler_cpu_get_thread();
void anscheduler_cpu_set_task(task_t * task);
void anscheduler_cpu_set_thread(thread_t * thread);
void anscheduler_cpu_notify_invlpg(task_t * task);
void anscheduler_cpu_notify_dead(task_t * task);
void anscheduler_cpu_stack_run(void * arg, void (* fn)(void * a));
void anscheduler_cpu_halt();
