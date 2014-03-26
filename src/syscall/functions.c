#include "functions.h"
#include <stdio.h>
#include <shared/addresses.h>
#include <anscheduler/functions.h>

static bool print_line(const char * ptr);

uint64_t syscall_entry(uint64_t arg1, uint64_t arg2, uint64_t arg3) {
  if (arg1 == 0) {
    syscall_print((void *)arg2);
  } else if (arg1 == 1) {
    return syscall_get_time();
  } else if (arg1 == 2) {
    syscall_sleep(arg2);
  } else if (arg1 == 3) {
    syscall_exit();
  }
  return 0;
}

void syscall_return(restore_regs * regs) {
  thread_t * thread = anscheduler_cpu_get_thread();
  task_t * task = thread->task;
  thread->state.rax = regs->rax;
  thread->state.rbp = regs->rbp;
  thread->state.rbx = regs->rbx;
  thread->state.rip = regs->rip;
  thread->state.rsp = regs->rsp;
  thread->state.cr3 = regs->cr3;
  thread->state.r12 = regs->r12;
  thread->state.r13 = regs->r13;
  thread->state.r14 = regs->r14;
  thread->state.r15 = regs->r15;
  thread->state.flags |= 0x200; // make sure interrupts are on
  thread->state.cs = 0x1b;
  thread->state.ss = 0x23;
  anscheduler_thread_run(task, thread);
}

void syscall_print(void * ptr) {
  // for each page worth of data, we need to go back and make sure it's mapped
  while (print_line(ptr)) {
    ptr += 0x50;
  }
}

uint64_t syscall_get_time() {
  anscheduler_cpu_lock();
  uint64_t ts = anscheduler_get_time();
  // provide a decent timestamp, but not perfect
  uint64_t res = 1000 * ts / (anscheduler_second_length() / 1000);
  anscheduler_cpu_unlock();
  return res;
}

void syscall_sleep(uint64_t usec) {
  anscheduler_cpu_lock();
  uint64_t units = (anscheduler_second_length() * usec) / 1000000L;
  uint64_t destTime = anscheduler_get_time() + units;
  thread_t * thread = anscheduler_cpu_get_thread();
  thread->nextTimestamp = destTime;
  anscheduler_loop_save_and_resign();
  anscheduler_cpu_unlock();
}

void syscall_exit() {
  anscheduler_cpu_lock();
  anscheduler_task_exit();
}

static bool print_line(const char * ptr) {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();

  int i;
  for (i = 0; i < 0x50; i++) {
    const char * addr = &ptr[i];
    uint64_t page = ((uint64_t)addr) >> 12;
    anscheduler_lock(&task->vmLock);
    uint16_t flags;
    uint64_t entry = anscheduler_vm_lookup(task->vm, page, &flags);
    anscheduler_unlock(&task->vmLock);
    if ((flags & 5) != 5) { // they're being sneaky, just stop printing
      anscheduler_cpu_unlock();
      return false;
    }
    page_t virPage = anscheduler_vm_virtual(entry);
    uint64_t virAddr = (((uint64_t)addr) & 0xfff) + (virPage << 12);
    char buff[2] = {*((const char *)virAddr), 0};
    if (buff[0] == 0) {
      anscheduler_cpu_unlock();
      return false;
    }
    print_lock();
    print(buff);
    print_unlock();
  }

  anscheduler_cpu_unlock();
  return true;
}

