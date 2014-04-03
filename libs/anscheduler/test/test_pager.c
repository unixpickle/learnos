#include "env/user_thread.h"
#include "env/threading.h"
#include "env/alloc.h"
#include "env/context.h"
#include <anscheduler/thread.h>
#include <anscheduler/task.h>
#include <anscheduler/loop.h>
#include <anscheduler/paging.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

static int taskCount = 2;

void proc_enter(void * unused);
void proc_empty_enter(void * unused);
void pager_thread();
void user_thread();
void user_thread_nokill();

void sys_poll();
void syscall_cont(void * unused);
void thread_poll_syscall(void * unused);

void * check_for_leaks(void * unused);

int main(int argc, const char ** argv) {
  antest_launch_thread(NULL, proc_enter);
  antest_launch_thread(NULL, proc_empty_enter);
  
  while (1) {
    sleep(100000);
  }
  return 0;
}

void proc_enter(void * unused) {
  // create pager thread
  task_t * task = anscheduler_task_create();
  anscheduler_task_launch(task);
  
  thread_t * thread = anscheduler_thread_create(task);
  antest_configure_user_thread(thread, pager_thread);
  
  anscheduler_thread_add(task, thread);
  anscheduler_task_dereference(task);
  
  anscheduler_loop_run();
}

void proc_empty_enter(void * unused) {
  anscheduler_loop_run();
}

void pager_thread() {
  anscheduler_cpu_lock();
  
  // set ourselves up to handle paging
  thread_t * thread = anscheduler_cpu_get_thread();
  anscheduler_pager_set(thread);
  
  int i, count = taskCount;
  for (i = 0; i < count; i++) {
    // create a user thread
    task_t * task = anscheduler_task_create();
    anscheduler_task_launch(task);
  
    thread = anscheduler_thread_create(task);
    antest_configure_user_thread(thread, user_thread);
  
    anscheduler_thread_add(task, thread);
    anscheduler_task_dereference(task);
  }
  
  anscheduler_cpu_unlock();
  
  bool mayDie = false;
  while (!mayDie) {
    sys_poll();
    page_fault_t * fault = anscheduler_pager_read();
    while (fault) {
      // terminate the other task
      if (fault->ptr != (void *)0x1337) {
        printf("[error] fault pointer wasn't 0x1337\n");
        exit(1);
      }
      anscheduler_cpu_lock();
      anscheduler_task_kill(fault->task, ANSCHEDULER_TASK_KILL_REASON_MEMORY);
      anscheduler_task_dereference(fault->task);
      anscheduler_free(fault);
      anscheduler_cpu_unlock();
      if (!__sync_sub_and_fetch(&taskCount, 1)) {
        mayDie = true;
      }
      fault = anscheduler_pager_read();
    }
  }
  
  anscheduler_cpu_lock();
  
  pthread_t athread;
  pthread_create(&athread, NULL, check_for_leaks, NULL);
  
  anscheduler_pager_set(NULL); // so we can get the hell out of here
  anscheduler_task_exit(0);
  anscheduler_cpu_unlock();
}

void user_thread() {
  printf("running user thread...\n");
  anscheduler_cpu_lock();
  anscheduler_page_fault((void *)0x1337, ANSCHEDULER_PAGE_FAULT_WRITE);
  anscheduler_cpu_unlock();
  printf("[error]: thread passed page fault!\n");
  exit(0);
}

/************
 * Syscalls *
 ************/

void sys_poll() {
  anscheduler_cpu_lock();
  anscheduler_save_return_state(anscheduler_cpu_get_thread(),
                                NULL, syscall_cont);
  anscheduler_cpu_unlock();
}

void syscall_cont(void * unused) {
  anscheduler_cpu_stack_run(NULL, thread_poll_syscall);
}

void thread_poll_syscall(void * unused) {
  task_t * task = anscheduler_cpu_get_task();
  if (!anscheduler_thread_poll()) {
    anscheduler_thread_run(task, anscheduler_cpu_get_thread());
  } else {
    anscheduler_cpu_set_task(NULL);
    anscheduler_cpu_set_thread(NULL);
    anscheduler_task_dereference(task);
    anscheduler_loop_run();
  }
}

/*********
 * Other *
 *********/

void * check_for_leaks(void * unused) {
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
