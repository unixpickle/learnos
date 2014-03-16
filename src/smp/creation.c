#include "types.h"
#include "destruction.h"
#include "util.h"
#include "tasks.h"
#include "cpu.h"
#include "vm.h"
#include "context.h"
#include "scheduler.h"
#include <kernpage.h>
#include <libkern_base.h>
#include <shared/addresses.h>
#include <anlock.h>

static void _generate_user_stack(void * rip);
static void _bootstrap_thread(void * program, uint64_t len);

static bool _create_4mb_identity(uint64_t * pml4);
static bool _allocate_user_code(void * program, uint64_t len);

task_t * task_alloc() {
  kernpage_lock();
  page_t taskPage = kernpage_alloc_virtual();
  page_t pml4 = kernpage_alloc_virtual();
  kernpage_unlock();

  if (!taskPage || !pml4) {
    kernpage_lock();
    if (pml4) kernpage_free_virtual(pml4);
    if (taskPage) kernpage_free_virtual(taskPage);
    kernpage_unlock();
    return NULL;
  }

  zero_page(pml4);
  zero_page(taskPage);

  task_t * task = (task_t *)(taskPage << 12);
  task->pml4 = kernpage_calculate_physical(pml4);
  task->pid = pids_next();
  task->isActive = true;

  if (!anidxset_initialize(&task->descriptors,
                           task_idxset_alloc,
                           task_idxset_free)) {
    kernpage_lock();
    kernpage_free_virtual(taskPage);
    kernpage_free_virtual(pml4);
    kernpage_unlock();
    return NULL;
  }

  if (!anidxset_initialize(&task->stacks,
                           task_idxset_alloc,
                           task_idxset_free)) {
    kernpage_lock();
    kernpage_free_virtual(taskPage);
    kernpage_free_virtual(pml4);
    kernpage_unlock();
    anidxset_free(&task->descriptors);
    return NULL;
  }

  if (!_create_4mb_identity((void *)(pml4 << 12))) {
    kernpage_lock();
    kernpage_free_virtual(taskPage);
    kernpage_free_virtual(pml4);
    kernpage_unlock();
    anidxset_free(&task->descriptors);
    anidxset_free(&task->stacks);
    return NULL;
  }

  return task;
}

thread_t * thread_alloc(task_t * task) {
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

  zero_page(mainPage);

  thread_t * thread = (thread_t *)(mainPage << 12);
  thread->isSystem = true;
  thread->task = task;
  anlock_lock(&task->stacksLock);
  thread->stack = anidxset_get(&task->stacks);
  anlock_unlock(&task->stacksLock);

  // map the kernel stack into the user space process
  anlock_lock(&task->pml4Lock);
  uint64_t entry = (kernpage_calculate_physical(kStack) << 12) | 3;
  page_t kernelStack = thread->stack + PROC_KERN_STACKS;
  task_vm_set(task, kernelStack, entry);
  task_vm_make_user(task, kernelStack);
  anlock_unlock(&task->pml4Lock);

  thread->state.rsp = (kStack << 12) + 0x1000;
  thread->state.rbp = (kStack << 12) + 0x1000;
  thread->state.cr3 = PML4_START;
  thread->state.flags = 0x200;

  return thread;
}

void thread_setup_first(thread_t * thread, void * program, uint64_t len) {
  thread->state.rdi = (uint64_t)program;
  thread->state.rsi = len;
  thread->state.rip = (uint64_t)_bootstrap_thread;
}

void thread_setup(thread_t * thread, void * rip) {
  thread->state.rdi = (uint64_t)rip;
  thread->state.rip = (uint64_t)_generate_user_stack;
}

void task_add_thread(task_t * task, thread_t * thread) {
  anlock_lock(&task->threadsLock);
  thread->nextThread = task->firstThread;
  if (task->firstThread) {
    task->firstThread->lastThread = thread;
  }
  task->firstThread = thread;
  anlock_unlock(&task->threadsLock);
}

bool task_generate(void * code, uint64_t length) {
  task_t * task = task_alloc();
  if (!task) return false;
  thread_t * thread = thread_alloc(task);
  if (!thread) {
    task_dealloc(task);
    return false;
  }
  thread_setup_first(thread, code, length);
  task_add_thread(task, thread);
  tasks_lock();
  tasks_add(task);
  tasks_unlock();

  task_queue_lock();
  task_queue_push(thread);
  task_queue_unlock();
  return true;
}

/**********************
 * Task bootstrapping *
 **********************/

static void _generate_user_stack(void * rip) {
  // perform memory allocation as needed
  disable_interrupts();
  cpu_t * cpu = cpu_current();
  task_t * task = cpu->task;
  thread_t * thread = cpu->thread;
  enable_interrupts();

  int i;
  page_t page = (thread->stack * 0x100) + PROC_USER_STACKS;
  for (i = 0; i < 0x100; i++) {
    disable_interrupts();
    kernpage_lock();
    page_t next = kernpage_alloc_virtual();
    kernpage_unlock();
    enable_interrupts();
    if (!next) {
      thread_exit();
      return;
    }
    uint64_t entry = 7 | (kernpage_calculate_physical(next) << 12);
    disable_interrupts();
    anlock_lock(&task->pml4Lock);
    bool result = task_vm_set(task, page, entry);
    task_vm_make_user(task, page);
    anlock_unlock(&task->pml4Lock);
    if (!result) {
      kernpage_lock();
      kernpage_free_virtual(next);
      kernpage_unlock();
      enable_interrupts();
      thread_exit();
      return;
    }
    enable_interrupts();
    page++;
  }

  disable_interrupts();
  thread->isSystem = false;
  if (!task->isActive) {
    thread_exit();
  }
  thread->state.rip = (uint64_t)rip;
  thread->state.rsp = (page << 12);
  thread->state.rbp = thread->state.rsp;
  thread->state.flags = 0x200; // interrupt flag
  thread->state.cr3 = (task->pml4 << 12);
  task_switch(task, thread);
}

static void _bootstrap_thread(void * program, uint64_t len) {
  if (!_allocate_user_code(program, len)) {
    thread_exit();
    return;
  }
  _generate_user_stack((void *)(PROC_CODE_BUFF << 12));
}

/***********
 * Utility *
 ***********/

static bool _create_4mb_identity(uint64_t * pml4) {
  kernpage_lock();
  page_t pdpt = kernpage_alloc_virtual();
  page_t pdt = kernpage_alloc_virtual();
  page_t pt1 = kernpage_alloc_virtual();
  page_t pt2 = kernpage_alloc_virtual();
  kernpage_unlock();
  if (!pdpt || !pdt || !pt1 || !pt2) {
    kernpage_lock();
    if (!pt2) kernpage_free_virtual(pt2);
    if (!pt1) kernpage_free_virtual(pt1);
    if (!pdt) kernpage_free_virtual(pdt);
    if (!pdpt) kernpage_free_virtual(pdpt);
    kernpage_unlock();
    return false;
  }
  zero_page(pdpt);
  zero_page(pdt);
  pml4[0] = (kernpage_calculate_physical(pdpt) << 12) | 3;
  uint64_t * table = (uint64_t *)(pdpt << 12);
  table[0] = (kernpage_calculate_physical(pdt) << 12) | 3;
  table = (uint64_t *)(pdt << 12);
  table[0] = (kernpage_calculate_physical(pt1) << 12) | 3;
  table[1] = (kernpage_calculate_physical(pt2) << 12) | 3;
  table = (uint64_t *)(pt1 << 12);
  uint64_t i;
  for (i = 0; i < 0x200; i++) {
    table[i] = (i << 12) | 3;
  }
  table = (uint64_t *)(pt2 << 12);
  for (i = 0; i < 0x200; i++) {
    table[i] = ((i + 0x200) << 12) | 3;
  }
  return true;
}

static bool _allocate_user_code(void * program, uint64_t len) {
  disable_interrupts();
  cpu_t * cpu = cpu_current();
  task_t * task = cpu->task;
  enable_interrupts();

  page_t i;
  page_t pageCount = (len >> 12) + ((len & 0xfffL) != 0);
  for (i = 0; i < pageCount; i++) {
    page_t page = i + PROC_CODE_BUFF;

    // allocate a page
    disable_interrupts();
    kernpage_lock();
    page_t newPage = kernpage_alloc_virtual();
    kernpage_unlock();
    enable_interrupts();
    if (!newPage) return false;

    // map the new page in
    uint64_t entry = 7 | (kernpage_calculate_physical(newPage) << 12);
    disable_interrupts();
    anlock_lock(&task->pml4Lock);
    bool result = task_vm_set(task, page, entry);
    task_vm_make_user(task, page);
    anlock_unlock(&task->pml4Lock);
    if (!result) {
      kernpage_lock();
      kernpage_free_virtual(newPage);
      kernpage_unlock();
      enable_interrupts();
      return false;
    }
    enable_interrupts();

    // copy the memory for this page
    uint8_t * source = (uint8_t *)(program + (i << 12));
    uint8_t * dest = (uint8_t *)(newPage << 12);
    uint64_t j, length = len - (i << 12);
    if (length > 0x1000) length = 0x1000;
    for (j = 0; j < length; j++) {
      dest[j] = source[j];
    }
  }
  return true;
}

