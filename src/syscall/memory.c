#include "memory.h"
#include "functions.h"
#include "vm.h"
#include <anscheduler/task.h>
#include <anscheduler/functions.h>
#include <anscheduler/paging.h>
#include <anscheduler/socket.h>
#include <anscheduler/loop.h>
#include <memory/kernpage.h>

static task_t * _get_remote_task(uint64_t fd);
static bool _vmmap_call(task_t * task, uint64_t virt, uint64_t entry);

uint64_t syscall_allocate_page() {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  if (task->uid) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }
  uint64_t res = kernpage_alloc_virtual();
  res = kernpage_calculate_physical(res) << 12;
  anscheduler_cpu_unlock();
  return res;
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

bool syscall_vmmap(uint64_t fd, uint64_t vpage, uint64_t entry) {
  anscheduler_cpu_lock();
  task_t * task = _get_remote_task(fd);
  if (!task) { 
    anscheduler_cpu_unlock();
    return false;
  }

  bool res = _vmmap_call(task, vpage, entry);
  anscheduler_task_dereference(task);
  anscheduler_cpu_unlock();
  
  return res;
}

bool syscall_vmunmap(uint64_t fd, uint64_t vpage) {
  anscheduler_cpu_lock();
  task_t * task = _get_remote_task(fd);
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

bool syscall_invlpg(uint64_t fd) {
  anscheduler_cpu_lock();
  task_t * task = _get_remote_task(fd);
  if (!task) {
    anscheduler_cpu_unlock();
    return false;
  }
  
  anscheduler_cpu_notify_invlpg(task);
  anscheduler_task_dereference(task);
  anscheduler_cpu_unlock();
  return true;
}

uint64_t syscall_self_vm_read(uint64_t vpage) {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  if (task->uid) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }

  uint16_t flags;
  anscheduler_lock(&task->vmLock);
  uint64_t page = anscheduler_vm_lookup(task->vm, vpage, &flags);
  anscheduler_unlock(&task->vmLock);
  anscheduler_cpu_unlock();

  return (page << 12) | flags;
}

void syscall_become_pager() {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  if (task->uid != 0) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }
  thread_t * thread = anscheduler_cpu_get_thread();
  anscheduler_pager_set(thread);
  anscheduler_cpu_unlock();
}

uint64_t syscall_get_fault(syscall_pg_t * pg) {
  anscheduler_cpu_lock();

  task_t * task = anscheduler_cpu_get_task();
  if (task->uid != 0) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }
  thread_t * thread = anscheduler_cpu_get_thread();
  if (thread != anscheduler_pager_get()) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }

  page_fault_t * fault = anscheduler_pager_peek();
  if (!fault) {
    anscheduler_cpu_unlock();
    return 0;
  }

  // copy out the information
  syscall_pg_t thePg;
  thePg.taskId = fault->task->pid;
  thePg.threadId = fault->thread->stack;
  thePg.address = (uint64_t)fault->ptr;
  thePg.flags = fault->flags;
  if (!task_copy_out(pg, &thePg, sizeof(syscall_pg_t))) {
    anscheduler_abort("system pager passed invalid ptr to syscall_get_fault");
  }

  anscheduler_cpu_unlock();
  return 1;
}

bool syscall_wake_thread(uint64_t fd, uint64_t tid) {
  anscheduler_cpu_lock();
  task_t * task = _get_remote_task(fd);
  if (!task) {
    anscheduler_cpu_unlock();
    return false;
  }
  
  anscheduler_lock(&task->threadsLock);
  thread_t * thread = task->firstThread;
  while (thread) {
    if (thread->stack == tid) break;
    thread = thread->next;
  }
  anscheduler_unlock(&task->threadsLock);
  anscheduler_loop_push(thread);

  anscheduler_task_dereference(task);
  anscheduler_cpu_unlock();
  return true;
}

bool syscall_self_vmmap(uint64_t vpage, uint64_t entry) {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  if (task->uid) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }
  bool res = _vmmap_call(task, vpage, entry);
  anscheduler_cpu_unlock();
  return res;
}

void syscall_self_vmunmap(uint64_t vpage) {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  if (task->uid) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }
  
  anscheduler_lock(&task->vmLock);
  anscheduler_vm_unmap(task->vm, vpage);
  anscheduler_unlock(&task->vmLock);
  
  anscheduler_cpu_unlock();
}

void syscall_self_invlpg() {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  if (task->uid) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }
  anscheduler_cpu_notify_invlpg(task);
  anscheduler_cpu_unlock();
}

void syscall_shift_fault() {
  anscheduler_cpu_lock();

  task_t * task = anscheduler_cpu_get_task();
  if (task->uid != 0) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }
  thread_t * thread = anscheduler_cpu_get_thread();
  if (thread != anscheduler_pager_get()) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }

  page_fault_t * f = anscheduler_pager_peek();
  if (f) {
    anscheduler_pager_shift();
    anscheduler_task_dereference(f->task);
    anscheduler_free(f);
  }
  anscheduler_cpu_unlock();
}

uint64_t syscall_mem_fault(uint64_t pid) {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  if (task->uid) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }
  task = anscheduler_task_for_pid(pid);
  if (!task) {
    anscheduler_cpu_unlock();
    return 0;
  }
  anscheduler_task_kill(task, ANSCHEDULER_TASK_KILL_REASON_MEMORY);
  anscheduler_task_dereference(task);
  anscheduler_cpu_unlock();
  return 1;
}

static task_t * _get_remote_task(uint64_t fd) {
  task_t * thisTask = anscheduler_cpu_get_task();
  if (!thisTask) return false;
  if (thisTask->uid) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }
  if (anscheduler_cpu_get_thread() != anscheduler_pager_get()) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }

  socket_desc_t * desc = anscheduler_socket_for_descriptor(fd);
  if (!desc) return NULL;
  task_t * remote = anscheduler_socket_remote(desc);
  anscheduler_socket_dereference(desc);
  return remote;
}

static bool _vmmap_call(task_t * task, uint64_t vpage, uint64_t entry) {
  anscheduler_lock(&task->vmLock);
  bool res = anscheduler_vm_map(task->vm, vpage, entry >> 12, entry & 0xfff);
  anscheduler_unlock(&task->vmLock);
  return res;
}
