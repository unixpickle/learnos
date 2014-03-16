#include "destruction.h"
#include "tasks.h"
#include "cpu.h"
#include "scheduler.h"
#include "vm.h"
#include "context.h"
#include <kernpage.h>
#include <libkern_base.h>
#include <shared/addresses.h>
#include <anlock.h>

static void _unlink_thread(void * threadObj);
static void _task_cleanup(task_t * task);

void thread_dealloc(thread_t * thread) {
  task_t * task = thread->task;

  anlock_lock(&task->pml4Lock);
  page_t procPage = thread->stack + PROC_KERN_STACKS;
  page_t phyPage = task_vm_lookup(task, procPage);
  anlock_unlock(&task->pml4Lock);

  uint64_t stackIndex = thread->stack;

  kernpage_lock();
  kernpage_free_virtual(((uint64_t)thread) >> 12);
  kernpage_free_virtual(kernpage_calculate_virtual(phyPage));
  kernpage_unlock();

  anlock_lock(&task->stacksLock);
  anidxset_put(&task->stacks, stackIndex);
  anlock_unlock(&task->stacksLock);
}

void task_dealloc(task_t * task) {
  kernpage_lock();
  kernpage_free_virtual(((page_t)task) >> 12);
  kernpage_unlock();
}

void thread_exit() {
  disable_interrupts();
  cpu_t * cpu = cpu_current();
  task_t * task = cpu->task;
  thread_t * thread = cpu->thread;
  thread->isSystem = true;
  enable_interrupts();

  // free up all the stack data that was allocated
  int i;
  page_t stackStart = (thread->stack * 0x100) + PROC_USER_STACKS;
  for (i = 0; i < 0x100; i++) {
    disable_interrupts();
    anlock_lock(&task->pml4Lock);
    page_t phyPage = task_vm_lookup(task, stackStart + i);
    anlock_unlock(&task->pml4Lock);
    if (phyPage & 1) {
      kernpage_lock();
      kernpage_free_virtual(kernpage_calculate_virtual(phyPage));
      kernpage_unlock();
    }
    enable_interrupts();
  }

  disable_interrupts();
  void * stack = (void *)((cpu->baseStack + 1) << 12);
  task_run_with_stack(stack, thread, _unlink_thread);
}

static void _unlink_thread(void * threadObj) {
  thread_t * thread = (thread_t *)threadObj;
  task_t * task = thread->task;

  // remove it from the doubly-linked list
  anlock_lock(&task->threadsLock);

  // here's where we'll quickly cleanup the major resources used by the task
  bool taskKilled = !thread->nextThread && task->firstThread == thread;
  if (taskKilled) {
    anlock_unlock(&task->threadsLock);
    enable_interrupts();
    _task_cleanup(task);
    disable_interrupts();
    anlock_lock(&task->threadsLock);
  }

  if (thread->nextThread) {
    thread->nextThread->lastThread = thread->lastThread;
  }
  if (thread->lastThread) {
    thread->lastThread->nextThread = thread->nextThread;
  } else {
    task->firstThread = thread->nextThread;
  }
  anlock_unlock(&task->threadsLock);
  thread_dealloc(thread);

  if (taskKilled) {
    uint64_t pid = task->pid;
    tasks_lock();
    tasks_remove(task);
    tasks_unlock();
    pids_release(pid);
  }

  scheduler_flush_timer();
  scheduler_task_loop();
}

static void _task_cleanup(task_t * task) {
  // Here, we will close all the task's sockets and free its heap

  disable_interrupts();
  task->isActive = false;
  enable_interrupts();

  // deallocate the code data
  page_t page = PROC_CODE_BUFF;
  for (; page < PROC_HEAP_BUFF; page++) {
    disable_interrupts();
    anlock_lock(&task->pml4Lock);
    uint64_t entry = task_vm_lookup(task, page);
    anlock_unlock(&task->pml4Lock);
    enable_interrupts();
    if (!entry) break;

    page_t vpage = kernpage_calculate_virtual(entry >> 12);
    disable_interrupts();
    kernpage_lock();
    kernpage_free_virtual(vpage);
    kernpage_unlock();
    enable_interrupts();
  }
}

