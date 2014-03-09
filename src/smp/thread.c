#include <kernpage.h>
#include <shared/addresses.h>
#include "thread.h"
#include "cpu_list.h"
#include "vm.h"
#include "context.h"
#include "scheduler.h"
#include <stdio.h>
#include <libkern_base.h>

typedef struct {
  task_t * task;
  thread_t * thread;
} __attribute__((packed)) task_thread_t;

/**
 * @discussion Enters a critical section itself, don't do it yourself.
 */
static void get_task_and_thread(task_thread_t * info);

static page_t _task_calculate_kernel_stack(uint64_t index);
static page_t _task_calculate_user_stack(uint64_t index);

/**
 * Enters a critical section periodically to allocate the virtual memory
 * in a task and copy the main executable code into it.
 */
static bool _allocate_user_code(void * program, uint64_t len);

/**
 * Copies memory from a source location to a destination page.
 */
static void _copy_memory(page_t dest, void * src, uint64_t len);

/**
 * Call this from a system thread to terminate the task. A reference is assumed
 * to both the `task` and `thread` arguments.
 * @discussion This function enters a critical section for you.
 */
static void _thread_unlink(task_t * task, thread_t * thread);

/**
 * This must be called from the critical section of a thread other than the
 * thread to be released, or else the stack will be deallocated while it's in
 * use. To get around this, _thread_unlink enters a critical section and then
 * switches to the CPU's stack -- that's what it's for, anyway.
 */
static void _release_thread(task_thread_t * info);

thread_t * thread_create_user(task_t * task, void * rip) {
  // allocate pages for the kernel stack and the thread's page
  kernpage_lock();
  page_t mainPage = kernpage_alloc_virtual();
  page_t kStack = kernpage_alloc_virtual();
  kernpage_unlock();

  if (!mainPage || !kStack) {
    kernpage_lock();
    if (mainPage) kernpage_free_virtual(mainPage);
    if (kStack) kernpage_free_virtual(kStack);
    kernpage_unlock();
    return NULL;
  }

  thread_t * thread = (thread_t *)(mainPage << 12);
  ref_initialize(&thread, (void (*)(void *))thread_dealloc);
  // todo: great things, here!
  thread->isSystem = true;
  thread->isRunning = 0;
  thread->nextThread = NULL;
  anlock_lock(&task->threadStacksLock);
  thread->stackIndex = anidxset_get(&task->threadStacks);
  anlock_unlock(&task->threadStacksLock);

  // map the kernel stack into the user space process
  anlock_lock(&task->pml4Lock);
  uint64_t entry = (kernpage_calculate_physical(kStack) << 12) | 3;
  page_t kernelStack = _task_calculate_kernel_stack(thread->stackIndex);
  task_vm_set(task, kernelStack, entry);
  task_vm_make_user(task, kernelStack);
  anlock_unlock(&task->pml4Lock);

  thread->state.rsp = (kStack << 12) + 0x1000;
  thread->state.rbp = (kStack << 12) + 0x1000;
  thread->state.cr3 = PML4_START;
  thread->state.rdi = (uint64_t)rip;
  thread->state.rip = (uint64_t)thread_configure_user_stack;
  thread->state.flags = 0;

  return thread;
}

thread_t * thread_create_first(task_t * task,
                               void * program,
                               uint64_t len) {
  thread_t * thread = thread_create_user(task, (void *)PROC_CODE_BUFF);
  if (!thread) return NULL;
  thread->state.rip = (uint64_t)thread_configure_user_program;
  thread->state.rsi = (uint64_t)program;
  thread->state.rdx = len;
  print("address of thread_configure_user_program: 0x");
  printHex((uint64_t)thread_configure_user_program);
  print("\n");
  return thread;
}

void * thread_resume_kernel_stack(task_t * task, thread_t * thread) {
  if (thread->state.cr3 == PML4_START) {
    return (void *)thread->state.rsp;
  } else {
    page_t vmStack = _task_calculate_kernel_stack(thread->stackIndex);
    anlock_lock(&task->pml4Lock);
    page_t physical = task_vm_lookup(task, vmStack);
    anlock_unlock(&task->pml4Lock);
    return (void *)((1 + kernpage_calculate_virtual(physical)) << 12);
  }
}

void thread_dealloc(thread_t * thread) {
  cpu_info * cpu = cpu_get_current();
  anlock_lock(&cpu->lock);
  task_t * task = (task_t *)ref_retain(cpu->currentTask);
  anlock_unlock(&cpu->lock);

  // get page of the kernel stack
  anlock_lock(&task->pml4Lock);
  page_t procPage = _task_calculate_kernel_stack(thread->stackIndex);
  page_t phyPage = task_vm_lookup(task, procPage);
  anlock_unlock(&task->pml4Lock);

  uint64_t stackIndex = thread->stackIndex;

  kernpage_lock();
  kernpage_free_virtual(((uint64_t)thread) >> 12);
  kernpage_free_virtual(kernpage_calculate_virtual(phyPage));
  kernpage_unlock();

  anlock_lock(&task->threadStacksLock);
  anidxset_put(&task->threadStacks, stackIndex);
  anlock_unlock(&task->threadStacksLock);

  ref_release(task);
}

void thread_configure_tss(thread_t * thread, tss_t * tss) {
  tss->rsp[0] = _task_calculate_kernel_stack(thread->stackIndex);
}

void thread_configure_user_stack(void * rip) {
  // enter and leave critical sections as we allocate memory
  task_thread_t ttt;
  get_task_and_thread(&ttt);
  task_t * task = ttt.task;
  thread_t * thread = ttt.thread;

  // allocate 0x100 pages
  int i;
  page_t pageIndex = _task_calculate_user_stack(thread->stackIndex);
  for (i = 0; i < 0x100; i++) {
    task_critical_start();
    kernpage_lock();
    page_t next = kernpage_alloc_virtual();
    kernpage_unlock();
    task_critical_stop();
    // if no page, kill the thread
    if (!next) _thread_unlink(task, thread); // will never return

    // attempt to map the page
    task_critical_start();
    anlock_lock(&task->pml4Lock);
    uint64_t entry = kernpage_calculate_physical(next) << 12;
    entry |= 7;
    bool result = task_vm_set(task, pageIndex, entry);
    task_vm_make_user(task, pageIndex);
    anlock_unlock(&task->pml4Lock);
    task_critical_stop();

    // if failed to map, free the page and unlink the thread
    if (!result) {
      task_critical_start();
      kernpage_lock();
      kernpage_free_virtual(next);
      kernpage_unlock();
      task_critical_stop();
      _thread_unlink(task, thread); // will never return
    }
    pageIndex++;
  }

  struct state_t state;
  state.rip = (uint64_t)rip;
  state.rsp = _task_calculate_user_stack(thread->stackIndex);
  state.rbp = state.rsp;
  state.flags = 0;
  state.cr3 = task->pml4;

  // we have retained both resources, so we must release them
  task_critical_start();
  ref_release(task);
  ref_release(thread);
  // TODO: jump to the state

  // this code is never reached, so there's no point
  // task_critical_stop();
}

void thread_configure_user_program(void * rip, void * program, uint64_t len) {
  __asm__ __volatile__ ("int $0x2");
  hang();
  // TODO: copy over all the user-space code here
  if (!_allocate_user_code(program, len)) {
    task_thread_t ttt;
    get_task_and_thread(&ttt);
    _thread_unlink(ttt.task, ttt.thread);
  }
  thread_configure_user_stack(rip);
}

static void get_task_and_thread(task_thread_t * info) {
  task_critical_start();
  cpu_info * cpu = cpu_get_current();
  anlock_lock(&cpu->lock);
  info->task = (task_t *)ref_retain(cpu->currentTask);
  info->thread = (thread_t *)ref_retain(cpu->currentThread);
  anlock_unlock(&cpu->lock);
  task_critical_stop();
}

static page_t _task_calculate_kernel_stack(uint64_t index) {
  return index + PROC_KERN_STACKS;
}

static page_t _task_calculate_user_stack(uint64_t index) {
  return (index * 0x100) + PROC_USER_STACKS;
}

static bool _allocate_user_code(void * program, uint64_t len) {
  task_critical_start();
  cpu_info * cpu = cpu_get_current();
  anlock_lock(&cpu->lock);
  task_t * task = (task_t *)ref_retain(cpu->currentTask);
  anlock_unlock(&cpu->lock);
  task_critical_stop();

  page_t pageIndex = PROC_CODE_BUFF;
  while (len) {
    task_critical_start();
    kernpage_lock();
    page_t next = kernpage_alloc_virtual();
    kernpage_unlock();
    if (!next) {
      ref_release(task);
      return false;
    }
    anlock_lock(&task->pml4Lock);
    uint64_t entry = (kernpage_calculate_physical(next) << 12) | 7;
    bool result = task_vm_set(task, pageIndex, entry);
    task_vm_make_user(task, pageIndex);
    anlock_unlock(&task->pml4Lock);
    task_critical_stop();
    if (!result) {
      task_critical_start();
      kernpage_lock();
      kernpage_free_virtual(next);
      kernpage_unlock();
      ref_release(task);
      task_critical_stop();
      return false;
    }
    uint64_t copyLen = len > 0x1000 ? 0x1000 : len;
    _copy_memory(next, program, copyLen);
    len -= copyLen;
    pageIndex++;
  }

  task_critical_start();
  ref_release(task);
  task_critical_stop();
  return true;
}

static void _copy_memory(page_t dest, void * src, uint64_t len) {
  uint8_t * buff = (uint8_t *)(dest << 12);
  uint8_t * source = (uint8_t *)source;
  uint64_t i;
  for (i = 0; i < len; i++) {
    buff[i] = source[i];
  }
}

static void _thread_unlink(task_t * task, thread_t * thread) {
  // free up all the stack data that was allocated
  int i;
  page_t stackStart = _task_calculate_user_stack(thread->stackIndex);
  for (i = 0; i < 0x100; i++) {
    task_critical_start();
    anlock_lock(&task->pml4Lock);
    page_t phyPage = task_vm_lookup(task, stackStart + i);
    anlock_unlock(&task->pml4Lock);
    if (phyPage) {
      kernpage_lock();
      kernpage_free_virtual(kernpage_calculate_virtual(phyPage));
      kernpage_unlock();
    }
    task_critical_stop();
  }

  // remove the thread from the task's thread linked-list
  task_thread_t info;
  info.task = task;
  info.thread = thread;

  task_critical_start();
  cpu_info * cpuInfo = cpu_get_current();
  void * stack = (void *)((cpuInfo->baseStack << 12) + 0x1000);
  task_run_with_stack(stack, &info, (void (*)(void *))_release_thread);
}

static void _release_thread(task_thread_t * _info) {
  task_thread_t info = *_info;
  // remove the thread from the list
  thread_t * prevThread = NULL;
  thread_t * thread = NULL;

  // remove the thread from the linked list
  anlock_lock(&info.task->threadsLock);

  // make sure the next thread in the task's thread queue isn't
  // the current thread.
  if (info.task->nextThread == info.thread) {
    info.task->nextThread = info.thread->nextThread;
  }

  thread = (thread_t *)ref_retain(info.task->firstThread);
  while (thread && thread != info.thread) {
    ref_release(prevThread);
    prevThread = thread;
    thread = (thread_t *)ref_retain(thread->nextThread);
  }

  // it *should* still be in the damn list
  if (thread) {
    if (prevThread) {
      prevThread->nextThread = thread->nextThread;
    } else {
      info.task->firstThread = thread->nextThread;
    }
  }
  ref_release(thread);
  ref_release(prevThread);
  anlock_unlock(&info.task->threadsLock);

  ref_release(info.thread);
  ref_release(info.task);

  // run a different thread than the one we just killed.
  scheduler_run_next();
}

