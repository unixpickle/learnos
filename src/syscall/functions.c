#include "functions.h"
#include "sockets.h"
#include "vm.h"
#include "io.h"
#include "exec.h"
#include <stdio.h>
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
  if (arg1 == 0) {
    syscall_print((void *)arg2);
  } else if (arg1 == 1) {
    return syscall_get_time();
  } else if (arg1 == 2) {
    syscall_sleep(arg2);
  } else if (arg1 == 3) {
    syscall_exit();
  } else if (arg1 == 4) {
    syscall_thread_exit();
  } else if (arg1 == 5) {
    syscall_wants_interrupts();
  } else if (arg1 == 6) {
    return syscall_get_interrupts();
  } else if (arg1 == 7) {
    return syscall_open_socket();
  } else if (arg1 == 8) {
    return syscall_connect(arg2, arg3);
  } else if (arg1 == 9) {
    syscall_close_socket(arg2);
  } else if (arg1 == 10) {
    return syscall_write(arg2, arg3, arg4);
  } else if (arg1 == 11) {
    return syscall_read(arg2, arg3);
  } else if (arg1 == 12) {
    return syscall_poll();
  } else if (arg1 == 13) {
    return syscall_remote_pid(arg2);
  } else if (arg1 == 14) {
    return syscall_remote_uid(arg2);
  } else if (arg1 == 15) {
    return syscall_in(arg2, arg3);
  } else if (arg1 == 16) {
    syscall_out(arg2, arg3, arg4);
  } else if (arg1 == 17) {
    syscall_set_color((uint8_t)arg2);
  } else if (arg1 == 18) {
    return syscall_fork(arg2);
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
  anscheduler_task_exit(0);
}

void syscall_thread_exit() {
  anscheduler_cpu_lock();
  anscheduler_thread_exit();
}

void syscall_wants_interrupts() {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  if (task->uid) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }
  anscheduler_set_interrupt_thread(anscheduler_cpu_get_thread());
  anscheduler_cpu_unlock();
}

uint64_t syscall_get_interrupts() {
  anscheduler_cpu_lock();
  thread_t * thread = anscheduler_cpu_get_thread();
  uint64_t result = __sync_fetch_and_and(&thread->irqs, 0);
  anscheduler_cpu_unlock();
  return result;
}

void syscall_set_color(uint8_t arg) {
  printColor(arg);
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

