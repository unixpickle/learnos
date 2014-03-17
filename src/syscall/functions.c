#include "functions.h"
#include <stdio.h>
#include <kernpage.h>
#include <smp/vm.h>
#include <smp/context.h>
#include <smp/scheduler.h>
#include <smp/timer.h>
#include <smp/cpu.h>
#include <smp/tasks.h>
#include <smp/destruction.h>
#include <interrupts/pit.h>
#include <shared/addresses.h>
#include <libkern_base.h>
#include <anlock.h>

static bool print_line(const char * ptr);

void syscall_print_method(void * ptr) {
  // for each page worth of data, we need to go back and make sure it's mapped
  enable_interrupts();
  while (print_line(ptr)) {
    ptr += 0x50;
  }
  disable_interrupts();
}

void syscall_sleep_method() {
  cpu_t * cpu = cpu_current();
  uint64_t time = cpu->thread->state.rdi;

  if (!time) {
    cpu->thread->state.rax = timer_get_time() / (timer_second_length() / 1000);
    task_switch(cpu->task, cpu->thread);
    return;
  }

  uint64_t cycles = (timer_second_length() / 1000) * time;
  cpu->thread->nextTimestamp = timer_get_time() + cycles;
  scheduler_stop_current();
  scheduler_task_loop();
}

void syscall_getint_method() {
  cpu_t * info = cpu_current();
  task_t * task = info->task;
  thread_t * thread = info->thread;
  uint64_t mask = __sync_fetch_and_and(&thread->interruptMask, 0);
  if (mask) {
    thread->state.rax = mask;
    task_switch(task, thread);
  } else {
    anlock_lock(&thread->statusLock);
    thread->status |= 2;
    anlock_unlock(&thread->statusLock);

    scheduler_stop_current();
    scheduler_task_loop();
  }
}

void syscall_thread_exit_method() {
  thread_exit();
}

void syscall_pid_kill_method(uint64_t pid) {
  // TODO: verify caller permissions
  tasks_lock();
  task_t * task = tasks_find(pid);
  bool isActive = task->isActive;
  tasks_unlock();
  if (isActive) task_kill(task);
}

static bool print_line(const char * ptr) {
  disable_interrupts();
  cpu_t * info = cpu_current();
  task_t * task = info->task;

  int i;
  for (i = 0; i < 0x50; i++) {
    const char * addr = &ptr[i];
    uint64_t page = ((uint64_t)addr) >> 12;
    anlock_lock(&task->pml4Lock);
    uint64_t entry = task_vm_lookup_raw(task, page);
    anlock_unlock(&task->pml4Lock);
    if ((entry & 5) != 5) { // they're being sneaky, just stop printing
      enable_interrupts();
      return false;
    }
    page_t virPage = kernpage_calculate_virtual(entry >> 12);
    uint64_t virAddr = (((uint64_t)addr) & 0xfff) + (virPage << 12);
    char buff[2] = {*((const char *)virAddr), 0};
    if (buff[0] == 0) {
      enable_interrupts();
      return false;
    }
    print_lock();
    print(buff);
    print_unlock();
  }

  enable_interrupts();
  return true;
}

