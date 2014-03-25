#include "user_thread.h"
#include "threading.h"
#include "general.h"
#include "vm.h"
#include "context.h"
#include "alloc.h"
#include <anscheduler/interrupts.h>
#include <anscheduler/thread.h>
#include <assert.h>

typedef struct {
  void * addr;
  uint64_t flags;
} __attribute__((packed)) page_fault_info;

static void _user_thread_entry(void * rip);
static void _free_old_stack(void * oldStack, void (* fn)());
static void _page_fault_cont(void * faultInfo);

void antest_configure_user_thread(thread_t * thread, void (* rip)()) {
  __asm__("pushfq\npop %0" : "=r" (thread->state.flags));
  thread->state.rip = (uint64_t)_user_thread_entry;
  // subtract 8 from the stack pointer because it needs to be aligned
  // for OS X
  thread->state.rsp = (uint64_t)anscheduler_alloc(0x1000) + 0xff8;
  thread->state.rbp = thread->state.rsp;
  thread->state.rdi = (uint64_t)rip;
  thread->state.cpuLocked = true;
}

void antest_user_thread_page_fault(void * addr, bool write) {
  // trigger a page fault an send it to the scheduler
  uint16_t flags = ANSCHEDULER_PAGE_FAULT_USER;
  if (write) flags |= ANSCHEDULER_PAGE_FAULT_WRITE;
  
  thread_t * thread = anscheduler_cpu_get_thread();
  
  page_fault_info info;
  info.addr = addr;
  info.flags = flags;
  anscheduler_save_return_state(thread, &info, _page_fault_cont);
}

void * antest_user_thread_stack_addr() {
  thread_t * thread = anscheduler_cpu_get_thread();
  return anscheduler_thread_user_stack(thread);
}

static void _user_thread_entry(void * rip) {
  // generate a page fault to allocate a page of user stack
  void * stackAddr = antest_user_thread_stack_addr() - 8;
  antest_user_thread_page_fault(stackAddr, true);
  
  // find the actual stack address
  task_t * task = anscheduler_cpu_get_task();
  anscheduler_lock(&task->vmLock);
  uint16_t flags;
  uint64_t vpage = ((uint64_t)stackAddr) >> 12;
  uint64_t entry = anscheduler_vm_lookup(task->vm, vpage, &flags);
  anscheduler_unlock(&task->vmLock);
  
  assert(flags & 1);
  
  void * newStack = (void *)((entry + 1) << 12);
  
  // free the stack we are using now
  uint64_t rsp;
  __asm__("mov %%rsp, %0" : "=r" (rsp));
  void * addr = (void *)(rsp & 0xfffffffffffff000L);
  
  // boom, jump right out of this place!
  __asm__("mov %0, %%rsp\ncallq *%1"
          : : "b" (newStack), "a" (_free_old_stack),
              "D" (addr), "S" (rip));
}

static void _free_old_stack(void * oldStack, void (* fn)()) {
  anscheduler_free(oldStack);
  antest_get_current_cpu_info()->isLocked = false;
  fn();
}

static void _page_fault_cont(void * faultInfo) {
  page_fault_info info = *((page_fault_info *)faultInfo);
  anscheduler_page_fault(info.addr, info.flags);
}
