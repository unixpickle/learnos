/**
 * Tests the basic functionality of message sending and the dynamics of
 * sockets. Ensures that, when a task exits, its sockets are closed. This does
 * NOT test for race conditions. Those will be tested somewhere else.
 */

#include "env/user_thread.h"
#include "env/threading.h"
#include "env/alloc.h"
#include "env/context.h"
#include <anscheduler/thread.h>
#include <anscheduler/task.h>
#include <anscheduler/loop.h>
#include <anscheduler/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>

typedef struct {
  // closer stage:
  // 0 - nothing has happened
  // 1 - got message from closer, closerFd is set
  // 2 - closer closed; now closerFd is invalidated
  int closerStage;
  uint64_t closerFd;
  
  int doneClosee; // 1 if done
  int gotKeepalive; // 1 if keepalive has been received
} server_scope;

void proc_enter(void * flag);
void create_thread(void (* method)());
void server_thread();
void client_closer_thread();
void client_closee_thread();
void client_keepalive_thread();
void * check_for_leaks(void * arg);

void syscall_cont(void * unused);
void thread_poll_syscall(void * unused);
socket_desc_t * connect_to_server();

void read_messages(server_scope * scope);
void handle_message(server_scope * scope, uint64_t fd, socket_msg_t * msg);

int main() {
  // create two CPUs for balanced testing
  antest_launch_thread(NULL, proc_enter);
  antest_launch_thread((void *)1, proc_enter);
  
  while (1) {
    sleep(0xffffffff);
  }
  
  return 0;
}

void proc_enter(void * flag) {
  if (flag) {
    create_thread(server_thread);
    create_thread(client_closer_thread);
    create_thread(client_closee_thread);
    create_thread(client_keepalive_thread);
  }
  anscheduler_loop_run();
}

void create_thread(void (* method)()) {
  char useless[2];
  task_t * task = anscheduler_task_create(useless, 2);
  anscheduler_task_launch(task);
  
  thread_t * thread = anscheduler_thread_create(task);
  antest_configure_user_thread(thread, method);
  
  anscheduler_thread_add(task, thread);
  anscheduler_task_dereference(task);
}

void server_thread() {
  server_scope scope;
  bzero(&scope, sizeof(scope));
  
  // here, listen for sockets
  anscheduler_cpu_lock();
  thread_t * thread = anscheduler_cpu_get_thread();
  anscheduler_cpu_unlock();
  while (1) {
    anscheduler_cpu_lock();
    anscheduler_save_return_state(thread, NULL, syscall_cont);
    anscheduler_cpu_unlock();
    read_messages(&scope);
    if (scope.closerStage == 2 && scope.doneClosee && scope.gotKeepalive) {
      printf("server shutting down...\n");
      anscheduler_cpu_lock();
      anscheduler_task_exit(0);
    }
  }
}

void client_closer_thread() {
  // here, open a socket, send "I don't love you", and close it
  anscheduler_cpu_lock();
  socket_desc_t * desc = connect_to_server();
  uint64_t fd = desc->descriptor;
  
  socket_msg_t * msg = anscheduler_socket_msg_data("I don't love you", 0x10);
  bool result = anscheduler_socket_msg(desc, msg);
  assert(result);
  
  // this is a new critical section
  desc = anscheduler_socket_for_descriptor(fd);
  assert(desc != NULL);
  anscheduler_socket_close(desc, 0);
  anscheduler_socket_dereference(desc);
  
  desc = anscheduler_socket_for_descriptor(fd);
  assert(desc == NULL);
  
  printf("closer done.\n");
  anscheduler_task_exit(0);
}

void client_closee_thread() {
  // here, open a socket, send "you wanna go on a date?", and wait for close
  anscheduler_cpu_lock();
  socket_desc_t * desc = connect_to_server();
  uint64_t fd = desc->descriptor;
  
  socket_msg_t * msg = anscheduler_socket_msg_data("you wanna go on a date?",
                                                   0x17);
  bool result = anscheduler_socket_msg(desc, msg);
  assert(result);
  
  thread_t * thread = anscheduler_cpu_get_thread();
  anscheduler_save_return_state(thread, NULL, syscall_cont);
  anscheduler_cpu_unlock();
  
  anscheduler_cpu_lock();
  // TODO: here, verify that the other end hung up
  desc = anscheduler_socket_for_descriptor(fd);
  anscheduler_socket_close(desc, 0);
  anscheduler_socket_dereference(desc);
  
  printf("closee done\n");
  anscheduler_task_exit(0);
}

void client_keepalive_thread() {
  // here, open a socket, send "marry me", and wait for it to close
  anscheduler_cpu_lock();
  socket_desc_t * desc = connect_to_server();
  uint64_t fd = desc->descriptor;
  
  socket_msg_t * msg = anscheduler_socket_msg_data("marry me", 8);
  bool result = anscheduler_socket_msg(desc, msg);
  assert(result);
  
  thread_t * thread = anscheduler_cpu_get_thread();
  anscheduler_save_return_state(thread, NULL, syscall_cont);
  anscheduler_cpu_unlock();
  
  anscheduler_cpu_lock();
  // TODO: here, verify that the other end hung up
  desc = anscheduler_socket_for_descriptor(fd);
  anscheduler_socket_close(desc, 0);
  anscheduler_socket_dereference(desc);
  
  pthread_t athread;
  pthread_create(&athread, NULL, check_for_leaks, NULL);
  printf("keepalive done\n");
  anscheduler_task_exit(0);
}

void * check_for_leaks(void * arg) {
  sleep(1);
  // one PID pool + 2 CPU stacks = 3 pages!
  if (antest_pages_alloced() != 3) {
    fprintf(stderr, "leaked 0x%llx pages\n",
            (unsigned long long)antest_pages_alloced() - 3);
    exit(1);
  }
  printf("test passed!\n");
  exit(0);
  return NULL;
}

void syscall_cont(void * unused) {
  anscheduler_cpu_stack_run(NULL, thread_poll_syscall);
}

void thread_poll_syscall(void * unused) {
  task_t * task = anscheduler_cpu_get_task();
  if (!anscheduler_thread_poll()) {
    anscheduler_thread_run(task, anscheduler_cpu_get_thread());
  } else {
    anscheduler_cpu_set_task(NULL);
    anscheduler_cpu_set_thread(NULL);
    anscheduler_task_dereference(task);
    anscheduler_loop_run();
  }
}

socket_desc_t * connect_to_server() {
  socket_desc_t * desc = anscheduler_socket_new();
  uint64_t fd = desc->descriptor;
  task_t * target = anscheduler_task_for_pid(0);
  assert(target != NULL);
  assert(target != anscheduler_cpu_get_task());
  bool result = anscheduler_socket_connect(desc, target);
  assert(result);
  
  // this is a new critical section
  desc = anscheduler_socket_for_descriptor(fd);
  assert(desc != NULL);
  return desc;
}

void read_messages(server_scope * scope) {
  while (1) {
    anscheduler_cpu_lock();
    socket_desc_t * desc = anscheduler_socket_next_pending();
    
    if (!desc) {
      anscheduler_cpu_unlock();
      return;
    }
    
    socket_msg_t * msg = anscheduler_socket_read(desc);
    while (msg) {
      handle_message(scope, desc->descriptor, msg);
      anscheduler_free(msg);
      msg = anscheduler_socket_read(desc);
    }
    
    anscheduler_socket_dereference(desc);
    anscheduler_cpu_unlock();
  }
}

void handle_message(server_scope * scope, uint64_t fd, socket_msg_t * msg) {
  printf("fd 0x%llx got type=0x%llx, len=0x%llx\n",
         (unsigned long long)fd, (unsigned long long)msg->type,
         (unsigned long long)msg->len);
  
  // initial closer
  if (msg->type == 1 && msg->len == 0x10 && !scope->closerStage) {
    if (!memcmp("I don't love you", (const char *)msg->message, 0x10)) {
      scope->closerStage = 1;
      scope->closerFd = fd;
    }
  } else if (scope->closerStage == 1 && fd == scope->closerFd) {
    assert(msg->type == 2);
    scope->closerStage = 2;
    
    socket_desc_t * desc = anscheduler_socket_for_descriptor(fd);
    assert(desc != NULL);
    anscheduler_socket_close(desc, 0);
    anscheduler_socket_dereference(desc);
  } else if (msg->len == 0x17) {
    if (!memcmp("you wanna go on a date?",
                (const char *)msg->message, 0x17)) {
      // close the bastard downnnnnn
      // TODO: send big fat "NO" message here! lololol
      socket_desc_t * desc = anscheduler_socket_for_descriptor(fd);
      assert(desc != NULL);
      anscheduler_socket_close(desc, 0);
      anscheduler_socket_dereference(desc);
      scope->doneClosee = true;
    }
  } else if (msg->len == 8) {
    if (!memcmp("marry me", (const char *)msg->message, 8)) {
      scope->gotKeepalive = 1;
    }
  }
}
