#include <anscheduler/task.h>
#include <anscheduler/thread.h>
#include <anscheduler/socket.h>
#include <anscheduler/functions.h>
#include <syscall/config.h>
#include <scheduler/context.h>

uint64_t syscall_fork(uint64_t rip) {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  task_t * fork = anscheduler_task_create();
  fork->ui.code = code_retain(task->ui.code);
  anscheduler_task_launch(fork);

  if (!fork) {
    anscheduler_abort("failed to fork task");
  }

  // open a stdio socket
  socket_desc_t * desc = anscheduler_socket_new();
  uint64_t fd = desc->descriptor;
  if (desc) {
    anscheduler_task_reference(fork);
    // socket_connect will not trigger a task switch because there are no
    // threads in the forked task.
    bool result = anscheduler_socket_connect(desc, fork);
    if (!result) {
      anscheduler_socket_close(desc, 0);
      anscheduler_socket_dereference(desc);
      anscheduler_task_dereference(fork);
      desc = NULL;
    }
  }
  if (!desc) {
    anscheduler_task_kill(fork, 0);
    anscheduler_task_dereference(fork);
    anscheduler_cpu_unlock();
    return 0xffffffffffffffffL;
  }

  thread_t * thread = anscheduler_thread_create(fork);
  uint64_t stackStart = ANSCHEDULER_TASK_USER_STACKS_PAGE
    + (thread->stack << 8);

  uint64_t cr3 = anscheduler_vm_physical(((uint64_t)fork->vm) >> 12) << 12;
  thread->state.cr3 = cr3;
  thread->state.rsp = (stackStart + 0x100) << 12;
  thread->state.rbp = (stackStart + 0x100) << 12;
  thread->state.flags = 0x200;
  thread->state.rip = rip;
  thread->state.cs = 0x1b;
  thread->state.ss = 0x23;
  syscall_initialize_thread(thread);
  thread_fxstate_init(thread);
  anscheduler_thread_add(fork, thread);

  anscheduler_task_dereference(fork);
  anscheduler_cpu_unlock();
  return fd;
}

bool syscall_kill(uint64_t pid) {
  anscheduler_cpu_lock();
  task_t * thisTask = anscheduler_cpu_get_task();
  task_t * task = anscheduler_task_for_pid(pid);
  if (!task) {
    anscheduler_cpu_unlock();
    return false;
  }
  if (thisTask->uid && thisTask->uid != task->uid) {
    anscheduler_cpu_unlock();
    return false;
  }
  anscheduler_task_kill(task, ANSCHEDULER_TASK_KILL_REASON_EXTERNAL);
  anscheduler_task_dereference(task);
  anscheduler_cpu_unlock();
  return true;
}

void syscall_thread_launch(uint64_t rip, uint64_t arg1, uint64_t arg2) {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  thread_t * thread = anscheduler_thread_create(task);
  uint64_t stackStart = ANSCHEDULER_TASK_USER_STACKS_PAGE
    + (thread->stack << 8);

  uint64_t cr3 = anscheduler_vm_physical(((uint64_t)task->vm) >> 12) << 12;
  thread->state.cr3 = cr3;
  thread->state.rsp = (stackStart + 0x100) << 12;
  thread->state.rbp = (stackStart + 0x100) << 12;
  thread->state.flags = 0x200;
  thread->state.rip = rip;
  thread->state.rdi = arg1;
  thread->state.rsi = arg2;
  thread->state.cs = 0x1b;
  thread->state.ss = 0x23;
  syscall_initialize_thread(thread);
  thread_fxstate_init(thread);
  anscheduler_thread_add(task, thread);
  anscheduler_cpu_unlock();
}

uint64_t syscall_thread_id() {
  anscheduler_cpu_lock();
  thread_t * thread = anscheduler_cpu_get_thread();
  uint64_t ident = thread->stack;
  anscheduler_cpu_unlock();
  return ident;
}

