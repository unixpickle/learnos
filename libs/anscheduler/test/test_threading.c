#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include <anscheduler/task.h>
#include <anscheduler/loop.h>
#include <anscheduler/thread.h>

#include "env/alloc.h"
#include "env/threading.h"
#include "env/user_thread.h"

void proc_enter(void * indicator);
void create_task(void (* method)());
void task_main();
void bg_method();
void * test_leaks(void * unused);

static uint64_t progCounter;

#define COUNTER_INIT 3

int main(int argc, const char * argv[]) {
  antest_launch_thread(NULL, proc_enter);
  antest_launch_thread((void *)1, proc_enter);
  while (1) sleep(0xffffffff);
  return 0;
}

void create_task(void (* method)()) {
  task_t * task = anscheduler_task_create();
  anscheduler_task_launch(task);
  
  thread_t * thread = anscheduler_thread_create(task);
  antest_configure_user_thread(thread, method);
  
  anscheduler_thread_add(task, thread);
  anscheduler_task_dereference(task);
}

void proc_enter(void * in) {
  if (in == NULL) {
    create_task(task_main);
  }
  anscheduler_loop_run();
}

void task_main() {
  int j, i;
  for (j = 0; j < 2; j++) {
    printf("spawning labor threads, attempt 0x%x...\n", j);
    // create some background threads
    uint64_t n = COUNTER_INIT;
    progCounter = COUNTER_INIT;
    for (i = 0; i < n; i++) {
      anscheduler_cpu_lock();
      task_t * task = anscheduler_cpu_get_task();
      thread_t * thread = anscheduler_thread_create(task);
      assert(thread->stack < i + COUNTER_INIT);
      antest_configure_user_thread(thread, bg_method);
      anscheduler_thread_add(task, thread);
      anscheduler_cpu_unlock();
    }
  
    volatile uint64_t * cnt = &progCounter;
    while (*cnt);
  }
  
  pthread_t thread;
  pthread_create(&thread, NULL, test_leaks, NULL);
  pthread_detach(thread);
  
  anscheduler_cpu_lock();
  anscheduler_task_exit(0);
}

void bg_method() {
  __sync_fetch_and_sub(&progCounter, 1);
  anscheduler_thread_exit();
}

void * test_leaks(void * unused) {
  sleep(1);
  printf("checking for leaks...\n");
  
  // one PID pool + 2 CPU stacks = 3 pages!
  if (antest_pages_alloced() != 3) {
    fprintf(stderr, "leaked 0x%llx pages\n",
            (unsigned long long)antest_pages_alloced() - 3);
    exit(1);
  }
  printf("test passed!\n");
  exit(0);
  
  return NULL;
}
