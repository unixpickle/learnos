#include "sockets.h"
#include "vm.h"
#include <anscheduler/functions.h>
#include <anscheduler/socket.h>
#include <anscheduler/loop.h>
#include <anscheduler/thread.h>
#include <anscheduler/task.h>

static void _poll_stub();
static void _poll_stub2();

uint64_t syscall_open_socket() {
  anscheduler_cpu_lock();
  socket_desc_t * desc = anscheduler_socket_new();
  uint64_t num = desc ? desc->descriptor : FD_INVAL;
  if (desc) anscheduler_socket_dereference(desc);
  anscheduler_cpu_unlock();
  return num;
}

uint64_t syscall_connect(uint64_t descNum, uint64_t pid) {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_task_for_pid(pid);
  if (!task) {
    anscheduler_cpu_unlock();
    return 0;
  }
  socket_desc_t * desc = anscheduler_socket_for_descriptor(descNum);
  if (!desc) {
    anscheduler_task_dereference(task);
    anscheduler_cpu_unlock();
    return 0;
  }
  bool result = anscheduler_socket_connect(desc, task);
  if (!result) {
    anscheduler_task_dereference(task);
    anscheduler_socket_dereference(desc);
  }
  anscheduler_cpu_unlock();
  return (uint64_t)result;
}

void syscall_close_socket(uint64_t desc) {
  anscheduler_cpu_lock();
  socket_desc_t * sock = anscheduler_socket_for_descriptor(desc);
  anscheduler_socket_close(sock, 0);
  anscheduler_socket_dereference(sock);
  anscheduler_cpu_unlock();
}

uint64_t syscall_write(uint64_t desc, uint64_t ptr, uint64_t len) {
  if (len > 0xfe8) return 0;

  anscheduler_cpu_lock();
  socket_desc_t * sock = anscheduler_socket_for_descriptor(desc);
  if (!sock) {
    anscheduler_cpu_unlock();
    return 0;
  }
  socket_msg_t * msg = anscheduler_alloc(0x1000);
  msg->type = ANSCHEDULER_MSG_TYPE_DATA;
  msg->len = len;
  if (!task_copy_in(msg->message, (void *)ptr, len)) {
    anscheduler_free(msg);
    anscheduler_socket_dereference(sock);
    anscheduler_cpu_unlock();
    return false;
  }
  bool result = anscheduler_socket_msg(sock, msg);
  if (!result) {
    anscheduler_free(msg);
    anscheduler_socket_dereference(sock);
  }
  anscheduler_cpu_unlock();
  return (uint64_t)result;
}

uint64_t syscall_read(uint64_t desc, uint64_t ptr) {
  anscheduler_cpu_lock();
  socket_desc_t * sock = anscheduler_socket_for_descriptor(desc);
  if (!sock) {
    anscheduler_cpu_unlock();
    return 0;
  }
  socket_msg_t * msg = anscheduler_socket_read(sock);
  anscheduler_socket_dereference(sock);
  if (!msg) {
    anscheduler_cpu_unlock();
    return 0;
  }
  return task_copy_out((void *)ptr, msg, 0x18 + msg->len);
}

uint64_t syscall_poll() {
  anscheduler_cpu_lock();
  thread_t * thread = anscheduler_cpu_get_thread();
  anscheduler_save_return_state(thread, NULL, _poll_stub);
  socket_desc_t * pending = anscheduler_socket_next_pending();
  uint64_t desc = FD_INVAL;
  if (pending) {
    desc = pending->descriptor;
    anscheduler_socket_dereference(pending);
  }
  anscheduler_cpu_unlock();
  return desc;
}

uint64_t syscall_remote_pid(uint64_t desc) {
  anscheduler_cpu_lock();
  socket_desc_t * sock = anscheduler_socket_for_descriptor(desc);
  if (!sock) {
    anscheduler_cpu_unlock();
    return 0xFFFFFFFFFFFFFFFFL;
  }
  task_t * remote = anscheduler_socket_remote(sock);
  anscheduler_socket_dereference(sock);
  uint64_t pid = remote->pid;
  anscheduler_task_dereference(remote);
  anscheduler_cpu_unlock();
  return pid;
}

uint64_t syscall_remote_uid(uint64_t desc) {
  anscheduler_cpu_lock();
  socket_desc_t * sock = anscheduler_socket_for_descriptor(desc);
  if (!sock) {
    anscheduler_cpu_unlock();
    return 0xFFFFFFFFFFFFFFFFL;
  }
  task_t * remote = anscheduler_socket_remote(sock);
  anscheduler_socket_dereference(sock);
  uint64_t uid = remote->uid;
  anscheduler_task_dereference(remote);
  anscheduler_cpu_unlock();
  return uid;
}

static void _poll_stub() {
  anscheduler_cpu_stack_run(NULL, _poll_stub2);
}

static void _poll_stub2() {
  if (!anscheduler_thread_poll()) {
    // resume the thing right away
    anscheduler_thread_run(anscheduler_cpu_get_task(),
                           anscheduler_cpu_get_thread());
  } else {
    anscheduler_loop_run();
  }
}

