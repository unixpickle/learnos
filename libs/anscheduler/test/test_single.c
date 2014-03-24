/**
 * Test that the scheduler can run a single thread.
 */

#include "env/user_thread.h"
#include "env/threading.h"
#include <anscheduler/thread.h>
#include <anscheduler/task.h>
#include <anscheduler/loop.h>
#include <stdio.h>
#include <unistd.h>

void proc_enter(void * unused);
void user_thread_test();

int main() {
  // create one CPU
  antest_launch_thread(NULL, proc_enter);
  
  while (1) {
    sleep(0xffffffff);
  }
  
  return 0;
}

void proc_enter(void * unused) {
  printf("launching task...\n");
  char useless[2];
  task_t * task = anscheduler_task_create(useless, 2);
  anscheduler_task_launch(task);
  
  printf("creating thread...\n");
  thread_t * thread = anscheduler_thread_create(task);
  antest_configure_user_thread(thread, user_thread_test);
  
  printf("scheduling...\n");
  anscheduler_thread_add(task, thread);
  anscheduler_task_dereference(task);
  
  printf("entering CPU loop...\n");
  anscheduler_loop_run();
}

void user_thread_test() {
  printf("test passed!\n");
  exit(0);
}
