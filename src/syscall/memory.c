#include "memory.h"
#include "functions.h"
#include <anscheduler/task.h>
#include <anscheduler/functions.h>
#include <memory/kernpage.h>

uint64_t syscall_allocate_page() {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  if (task->uid) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }
  uint64_t res = kernpage_alloc_virtual();
  anscheduler_cpu_unlock();
  return kernpage_calculate_physical(res) << 12;
}

uint64_t syscall_allocate_aligned(uint64_t pages) {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  if (task->uid) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }
  uint64_t res = kernpage_alloc_pci(pages);
  anscheduler_cpu_unlock();
  return res;
}

void syscall_free_page(uint64_t addr) {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  if (task->uid) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }
  kernpage_free_virtual(kernpage_calculate_virtual(addr >> 12));
  anscheduler_cpu_unlock();
}

void syscall_free_aligned(uint64_t addr, uint64_t size) {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  if (task->uid) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }
  kernpage_free_pci(addr, size);
  anscheduler_cpu_unlock();
}

bool syscall_vmmap(uint64_t pid, uint64_t vpage, uint64_t entry) {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  if (task->uid) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }
  task = anscheduler_task_for_pid(pid);
  if (!task) {
    anscheduler_cpu_unlock();
    return false;
  }

  anscheduler_lock(&task->vmLock);
  bool res = anscheduler_vm_map(task->vm, vpage, entry >> 12, entry & 0xfff);
  anscheduler_unlock(&task->vmLock);
  anscheduler_task_dereference(task);

  anscheduler_cpu_unlock();
  return res;
}

bool syscall_vmunmap(uint64_t pid, uint64_t vpage) {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  if (task->uid) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }
  task = anscheduler_task_for_pid(pid);
  if (!task) {
    anscheduler_cpu_unlock();
    return false;
  }

  anscheduler_lock(&task->vmLock);
  anscheduler_vm_unmap(task->vm, vpage);
  anscheduler_unlock(&task->vmLock);
  anscheduler_task_dereference(task);

  anscheduler_cpu_unlock();
  return true;
}

bool syscall_invlpg(uint64_t pid) {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  if (task->uid) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }
  task = anscheduler_task_for_pid(pid);
  if (!task) {
    anscheduler_cpu_unlock();
    return false;
  }

  // while we may be *the* task, it doesn't matter because the interrupt will
  // not arrive until after anscheduler_cpu_unlock().
  anscheduler_cpu_notify_invlpg(task);
  anscheduler_task_dereference(task);
  anscheduler_cpu_unlock();
  return true;
}

