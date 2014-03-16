#include "lifecycle.h"
#include "vm.h"
#include "tasks.h"
#include "scheduler.h"
#include <shared/addresses.h>
#include <kernpage.h>
#include <anlock.h>

static void _mask_interrupt(uint64_t mask, task_t * task);

void thread_configure_tss(thread_t * thread, tss_t * tss) {
  uint64_t kStack = thread->stack + PROC_KERN_STACKS;
  tss->rsp[0] = ((kStack + 1) << 12);
}

void * thread_resume_kernel_stack(task_t * task, thread_t * thread) {
  if (thread->state.cr3 == PML4_START) {
    return (void *)thread->state.rsp - 0x80; // respect the Red Zone
  } else {
    page_t vmStack = thread->stack + PROC_KERN_STACKS;
    anlock_lock(&task->pml4Lock);
    page_t physical = task_vm_lookup(task, vmStack);
    anlock_unlock(&task->pml4Lock);
    return (void *)((1 + kernpage_calculate_virtual(physical)) << 12);
  }
}

void * thread_translate_kernel_stack(task_t * task,
                                     thread_t * thread,
                                     void * ptr) {
  page_t localPage = thread->stack + PROC_KERN_STACKS;
  return (void *)((localPage << 12) | (((uint64_t)ptr) & 0xfff));
}

void scheduler_handle_interrupt(uint64_t irqMask) {
  tasks_lock();
  tasks_iterate(irqMask, _mask_interrupt);
  tasks_unlock();
}

static void _mask_interrupt(uint64_t mask, task_t * task) {
  anlock_lock(&task->threadsLock);
  thread_t * thread = task->firstThread;
  while (thread) {
    anlock_lock(&thread->statusLock);
    if (!(thread->status &= 0xd)) {
      thread->state.rax = mask;
      task_queue_lock();
      task_queue_push(thread);
      task_queue_unlock();
    } else {
      __asm__ ("lock or %0, (%1)"
               : : "a" (mask), "b" (&thread->interruptMask)
               : "memory");
    }
    anlock_unlock(&thread->statusLock);
    thread = thread->nextThread;
  }
  anlock_unlock(&task->threadsLock);
}

