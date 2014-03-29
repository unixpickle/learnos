#include <anscheduler/task.h>
#include <anscheduler/thread.h>
#include <anscheduler/socket.h>
#include <anscheduler/functions.h>
#include <syscall/config.h>

uint64_t syscall_fork(uint64_t rip) {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  task_t * fork = anscheduler_task_fork(task);
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
  anscheduler_thread_add(fork, thread);

  anscheduler_task_dereference(fork);
  anscheduler_cpu_unlock();
  return fd;
}

