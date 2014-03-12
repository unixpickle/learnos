#include "functions.h"
#include <stdio.h>
#include <kernpage.h>
#include <smp/vm.h>
#include <smp/cpu_list.h>
#include <interrupts/pit.h>
#include <shared/addresses.h>
#include <smp/scheduler.h>

static bool print_line(const char * ptr);

void syscall_print_method(void * ptr) {
  // for each page worth of data, we need to go back and make sure it's mapped
  task_critical_stop();
  while (print_line(ptr)) {
    ptr += 0x50;
  }
  task_critical_start();
}

void syscall_sleep_method(uint64_t time) {
  task_critical_stop();
  pit_sleep(time);
  task_critical_start();
}

void syscall_getint_method() {
  cpu_info * info = cpu_get_current();
  anlock_lock(&info->lock);
  thread_t * thread = info->currentThread;
  anlock_unlock(&info->lock);
  if (thread) __sync_fetch_and_or(&thread->runState, 2);
  scheduler_run_next();
}

static bool print_line(const char * ptr) {
  task_critical_start();
  cpu_info * info = cpu_get_current();
  anlock_lock(&info->lock);
  task_t * task = ref_retain(info->currentTask);
  anlock_unlock(&info->lock);

  int i;
  for (i = 0; i < 0x50; i++) {
    const char * addr = &ptr[i];
    uint64_t page = ((uint64_t)addr) >> 12;
    anlock_lock(&task->pml4Lock);
    uint64_t entry = task_vm_lookup_raw(task, page);
    anlock_unlock(&task->pml4Lock);
    if ((entry & 5) != 5) { // they're being sneaky, just stop printing
      ref_release(task);
      task_critical_stop();
      return false;
    }
    page_t virPage = kernpage_calculate_virtual(entry >> 12);
    uint64_t virAddr = (((uint64_t)addr) & 0xfff) + (virPage << 12);
    char buff[2] = {*((const char *)virAddr), 0};
    if (buff[0] == 0) {
      ref_release(task);
      task_critical_stop();
      return false;
    }
    print_lock();
    print(buff);
    print_unlock();
  }

  ref_release(task);
  task_critical_stop();
  return true;
}

