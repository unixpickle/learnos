#include "threading.h"
#include "timer.h"
#include "interrupts.h"
#include "alloc.h"
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <anlock.h>
#include <stdlib.h>

typedef struct {
  void (* method)(void *);
  void * arg;
} newthread_args;

static uint64_t cpusLock = 0;
static cpu_info cpus[0x20];
static uint64_t cpuCount = 0;
__thread cpu_info * cpu;

static void * thread_enter(newthread_args * args);
static bool test_for_interrupt();

cpu_info * antest_get_current_cpu_info() {
  return cpu;
}

void antest_launch_thread(void * arg, void (* method)(void *)) {
  pthread_t thread;
  newthread_args * args = malloc(sizeof(newthread_args));
  args->method = method;
  args->arg = arg;
  pthread_create(&thread, NULL, (void * (*)(void *))thread_enter, args);
}

void anscheduler_cpu_lock() {
  assert(!cpu->isLocked);
  cpu->isLocked = true;
}

void anscheduler_cpu_unlock() {
  assert(cpu->isLocked);
  cpu->isLocked = false;
  
  if (test_for_interrupt()) {
    antest_handle_timer_interrupt();
  }
}

task_t * anscheduler_cpu_get_task() {
  return antest_get_current_cpu_info()->task;
}

thread_t * anscheduler_cpu_get_thread() {
  return antest_get_current_cpu_info()->thread;
}

void anscheduler_cpu_set_task(task_t * task) {
  antest_get_current_cpu_info()->task = task;
}

void anscheduler_cpu_set_thread(thread_t * thread) {
  antest_get_current_cpu_info()->thread = thread;
}

void anscheduler_cpu_notify_invlpg(task_t * task) {
  // nothing to do here since we don't actually do paging
}

void anscheduler_cpu_notify_dead(task_t * task) {
  int i;
  for (i = 0; i < cpuCount; i++) {
    if (cpus[i].task == task) {
      cpus[i].nextInterrupt = 0;
    }
  }
}

void anscheduler_cpu_stack_run(void * arg, void (* fn)(void * a)) {
  void * ptr = cpu->cpuStack + 0x1000;
  __asm__("mov %%rcx, %%rsp\n"
          "callq *%%rax"
          : : "c" (ptr), "D" (arg), "a" (fn));
}

void anscheduler_cpu_halt() {
  assert(!antest_get_current_cpu_info()->isLocked);
  while (!test_for_interrupt()) {
    usleep(1000);
  }
  
  antest_handle_timer_interrupt();
}

static void * thread_enter(newthread_args * _args) {
  newthread_args args = *_args;
  free(_args);
  
  anlock_lock(&cpusLock);
  cpu = &cpus[cpuCount++];
  anlock_unlock(&cpusLock);
  
  cpu->isLocked = true;
  cpu->task = NULL;
  cpu->thread = NULL;
  cpu->nextInterrupt = 0xffffffffffffffffL;
  cpu->cpuStack = anscheduler_alloc(0x1000);
  
  args.method(args.arg);
  
  anlock_lock(&cpusLock);
  cpuCount--;
  anlock_unlock(&cpusLock);
  return NULL;
}

static bool test_for_interrupt() {
  uint64_t now = anscheduler_get_time();
  if (cpu->nextInterrupt <= now) {
    anscheduler_timer_cancel();
    return true;
  }
  return false;
}
