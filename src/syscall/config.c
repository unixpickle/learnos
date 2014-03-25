#include "config.h"
#include "functions.h"
#include <scheduler/cpu.h>
#include <libkern_base.h>
#include <shared/addresses.h>
#include <anscheduler/thread.h>
#include <anscheduler/functions.h>
#include <string.h> // memcpy

void syscall_initialize() {
  uint64_t star = (8L << 32) | (0x18L << 48);
  msr_write(MSR_STAR, star);
  msr_write(MSR_SFMASK, 0); // interrupts will be enabled
}

void syscall_initialize_thread(thread_t * thread) {
  // use the maximal pages for everything because we're a bunch of ballers
  uint64_t pageIndex = 0xFFFFFFFFFL - thread->stack;

  // map the address in the task's address space to the thread structure
  task_t * task = thread->task;
  page_t physical = anscheduler_vm_physical(((uint64_t)thread) >> 12);
  anscheduler_lock(&task->vmLock);
  anscheduler_vm_map(task->vm, pageIndex, physical, 3);
  anscheduler_unlock(&task->vmLock);

  // setup the code structure (someone, please, just MURDER ME)
  memcpy(&thread->state.callCode.code1,
         "\x51\xFA\x41\x0F\x20\xDB\x49\x89\xE2\x48\xB8", 0xb);
  memcpy(&thread->state.callCode.code2,
         "\x48\x89\xC4\x48\xB8", 5);
  memcpy(&thread->state.callCode.code3,
         "\x0F\x22\xD8\xFB\x41\x52\x41\x53\x48\xB8", 0xa);
  memcpy(&thread->state.callCode.code4,
         "\xFF\xD0\xFA\x41\x5B\x41\x5A\x4C\x89\xD4"
         "\x41\x0F\x22\xDB\xFB\x59\x0F\x07",
         0x10);

  uint64_t stack = (uint64_t)anscheduler_thread_kernel_stack(task, thread);
  stack += 0x1000;

  //debugging: try doing nothing with `jmp $`
  //thread->state.callCode.code1[0] = 0xeb;
  //thread->state.callCode.code1[1] = 0xfe;

  print("interrupt stack is ");
  printHex(stack);
  print(", code start is ");
  printHex(((void *)&thread->state.callCode) - (void *)thread);
  print("\n");

  thread->state.callCode.newPML4 = PML4_START;
  thread->state.callCode.stack = stack;
  thread->state.callCode.routine = (uint64_t)&syscall_entry;
}

void syscall_setup_for_thread(thread_t * thread) {
  uint64_t page = 0xFFFFFFFFFL - thread->stack;
  uint64_t addr = page << 12;
  if (addr & (1L << 47)) {
    addr |= 0xffff000000000000;
  }
  thread_t * ptr = (thread_t *)addr;
  msr_write(MSR_LSTAR, (uint64_t)&ptr->state.callCode);
}

