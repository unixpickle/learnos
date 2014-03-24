#include "context.h"
#include "cpu.h"
#include <shared/addresses.h>
#include <anscheduler/task.h>
#include <anscheduler/functions.h>
#include <kernpage.h>
#include <syscall/config.h>

void anscheduler_thread_run(task_t * task, thread_t * thread) {
  cpu_t * cpu = cpu_current();
  tss_t * tss = cpu->tss;
  uint64_t kStack = thread->stack + ANSCHEDULER_TASK_KERN_STACKS_PAGE;
  tss->rsp[0] = ((kStack + 1) << 12);
  syscall_setup_for_thread(thread);
  thread_run_state(thread);
}

void anscheduler_set_state(thread_t * thread,
                           void * stack,
                           void * ip,
                           void * arg1) {
  anscheduler_zero(&thread->state, sizeof(anscheduler_state));
  thread->state.flags = 0x200; // interrupts enabled
  thread->state.cr3 = PML4_START;
  thread->state.rip = (uint64_t)ip;
  thread->state.rsp = (uint64_t)stack;
  thread->state.rdi = (uint64_t)arg1;
}

void * thread_resume_kernel_stack(thread_t * thread) {
  if (thread->state.cr3 == PML4_START) {
    return (void *)thread->state.rsp; // no Red Zone needed here!
  } else {
    task_t * task = thread->task;
    page_t vmStack = thread->stack + ANSCHEDULER_TASK_KERN_STACKS_PAGE;
    anscheduler_lock(&task->vmLock);
    uint16_t flags;
    page_t physical = anscheduler_vm_lookup(task->vm, vmStack, &flags);
    anscheduler_unlock(&task->vmLock);
    return (void *)((1L + kernpage_calculate_virtual(physical)) << 12);
  }
}

void * thread_vm_kernel_stack(thread_t * thread, void * ptr) {
  // TODO: try all this stuff without nasty casting
  page_t localPage = thread->stack + ANSCHEDULER_TASK_KERN_STACKS_PAGE;
  return (void *)((localPage << 12) | (((uint64_t)ptr) & 0xfff));
}

