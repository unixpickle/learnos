#include "functions.h"
#include "sockets.h"
#include "vm.h"
#include "io.h"
#include "exec.h"
#include "memory.h"
#include <stdio.h>
#include <memory/kernpage.h>
#include <shared/addresses.h>
#include <anscheduler/functions.h>
#include <anscheduler/task.h>
#include <anscheduler/thread.h>
#include <anscheduler/loop.h>
#include <anscheduler/interrupts.h>

static bool print_line(const char * ptr);

uint64_t syscall_entry(uint64_t arg1,
                       uint64_t arg2,
                       uint64_t arg3,
                       uint64_t arg4) {
  void * functions[] = {
    (void *)syscall_print, // 0x0
    (void *)syscall_get_time,
    (void *)syscall_sleep,
    (void *)syscall_exit,
    (void *)syscall_thread_exit,
    (void *)syscall_wants_interrupts,
    (void *)syscall_get_interrupts,
    (void *)syscall_open_socket,
    (void *)syscall_connect,
    (void *)syscall_close_socket,
    (void *)syscall_write,
    (void *)syscall_read,
    (void *)syscall_poll,
    (void *)syscall_remote_pid,
    (void *)syscall_remote_uid,
    (void *)syscall_in,
    (void *)syscall_out, // 0x10
    (void *)syscall_set_color,
    (void *)syscall_fork,
    (void *)syscall_mem_usage,
    (void *)syscall_kill,
    (void *)syscall_allocate_page,
    (void *)syscall_allocate_aligned,
    (void *)syscall_free_page,
    (void *)syscall_free_aligned,
    (void *)syscall_vmmap,
    (void *)syscall_vmunmap,
    (void *)syscall_invlpg,
    (void *)syscall_thread_launch,
    (void *)syscall_thread_id,
    (void *)syscall_unsleep,
    (void *)syscall_self_uid,
    (void *)syscall_self_pid, // 0x20
    (void *)syscall_self_vm_read,
    (void *)syscall_become_pager,
    (void *)syscall_get_fault,
    (void *)syscall_self_vmmap,
    (void *)syscall_self_vmunmap,
    (void *)syscall_self_invlpg,
    (void *)syscall_shift_fault,
    (void *)syscall_abort,
    (void *)syscall_mem_fault,
    (void *)syscall_wake_thread,
    (void *)syscall_batch_vmunmap,
    (void *)syscall_batch_alloc
  };
  if (arg1 >= sizeof(functions) / sizeof(void *)) {
    return 0;
  }
  uint64_t ret;
  void * func = functions[arg1];
  __asm__("xor %%rax, %%rax\n"
          "call *%%rcx"
          : "=a" (ret)
          : "D" (arg2), "S" (arg3), "d" (arg4), "c" (func)
          : "r8", "r9", "r10", "r11", "memory");
  return ret;
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
  anscheduler_task_exit(0);
}

void syscall_thread_exit() {
  anscheduler_thread_exit();
}

void syscall_wants_interrupts() {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  if (task->uid) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }
  anscheduler_intd_set(anscheduler_cpu_get_thread());
  anscheduler_cpu_unlock();
}

uint64_t syscall_get_interrupts() {
  anscheduler_cpu_lock();
  if (anscheduler_cpu_get_task()->uid) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }
  uint64_t result = anscheduler_intd_read();
  anscheduler_cpu_unlock();
  return result;
}

void syscall_set_color(uint8_t arg) {
  printColor(arg);
}

uint64_t syscall_mem_usage() {
  uint64_t count = kernpage_count_allocated();
  return count;
}

uint64_t syscall_self_uid() {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  uint64_t res = task->uid;
  anscheduler_cpu_unlock();
  return res;
}

uint64_t syscall_self_pid() {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  uint64_t res = task->pid;
  anscheduler_cpu_unlock();
  return res;
}

void syscall_abort() {
  anscheduler_cpu_lock();
  anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ABORT);
}

static bool print_line(const char * ptr) {
  anscheduler_cpu_lock();
  bool ret = true;

  int i;
  for (i = 0; i < 0x50; i++) {
    char buff[2] = {0, 0};
    bool result = task_copy_in(buff, ptr + i, 1);
    if (!result) {
      anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_MEMORY);
    }
    if (buff[0] == 0) {
      ret = false;
      break;
    }
    print(buff);
  }

  anscheduler_cpu_unlock();
  return ret;
}

