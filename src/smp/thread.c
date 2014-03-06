#include "thread.h"
#include "cpu_list.h"
#include "vm.h"
#include <kernpage.h>
#include <shared/addresses.h>

page_t _task_calculate_kernel_stack(uint64_t index);

thread_t * thread_create_user(task_t * task, void * rip) {
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
  thread->nextThread = NULL;
  anlock_initialize(&thread->nextThreadLock);
  anlock_lock(&task->threadStacksLock);
  thread->stackIndex = anidxset_get(&task->threadStacks);
  anlock_unlock(&task->threadStacksLock);

  // map the kernel stack into the user space process
  anlock_lock(&task->pml4Lock);
  uint64_t entry = (kernpage_calculate_physical(kStack) << 12) | 3;
  task_vm_set(task, _task_calculate_kernel_stack(thread->stackIndex), entry);
  anlock_unlock(&task->pml4Lock);

  thread->state.rsp = kStack << 12;
  thread->state.rbp = kStack << 12;
  thread->state.cr3 = PML4_START;
  thread->state.rdi = (uint64_t)rip;
  thread->state.rip = (uint64_t)thread_configure_user_stack;
  thread->state.flags = 0;

  return thread;
}

thread_t * thread_create_first(task_t * task,
                               void * rip,
                               void * program,
                               uint64_t len) {
  thread_t * thread = thread_create_user(task, rip);
  if (!thread) return NULL;
  thread->state.rip = (uint64_t)thread_configure_user_program;
  thread->state.rsi = (uint64_t)program;
  thread->state.rdx = len;
  return thread;
}

void thread_dealloc(thread_t * thread) {
  cpu_info * cpu = cpu_get_current();
  anlock_lock(&cpu->lock);
  task_t * task = (task_t *)ref_retain(cpu->currentTask);
  anlock_unlock(&cpu->lock);

  // get page
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

void thread_configure_user_stack(void * rip) {
  // NYI
}

void thread_configure_user_program(void * rip, void * program, uint64_t len) {
  // NYI
}

page_t _task_calculate_kernel_stack(uint64_t index) {
  return index + PROC_KERN_STACKS;
}

