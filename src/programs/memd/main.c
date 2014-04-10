#include "client.h"
#include <stdlib.h>
#include <stdio.h>

void handle_messages(uint64_t fd);
void handle_faults();

void handle_client_fault(client_t * cli, pgf_t * fault);

void main() {
  sys_become_pager();

  sys_sleep(0x100000);
  printf("allocating a buffer, usage is 0x%x\n", sys_mem_usage());
  char * buf = malloc(10);
  printf("buffer is 0x%x, mem usage is 0x%x\n", buf, sys_mem_usage());
  free(buf);
  printf("free'd buffer 0x%x\n", sys_mem_usage());

  uint64_t i;
  for (i = 0; i < 10; i++) {
    buf = malloc(10);
    printf("new buffer is 0x%x, mem usage is 0x%x\n", buf, sys_mem_usage());
    free(buf);
    printf("free'd buffer 0x%x\n", sys_mem_usage());
  }
  for (i = 0; i < 10; i++) {
    sbrk(1);
    printf("broke one 0x%x\n", sys_mem_usage());
    sbrk(-1L);
    printf("broke minus one 0x%x\n", sys_mem_usage());
  }

  while (1) {
    uint64_t fd = sys_poll();
    if (fd + 1) handle_messages(fd);
    handle_faults();
  }
}

void handle_messages(uint64_t fd) {
  msg_t msg;
  while (sys_read(fd, &msg)) {
    // TODO: something
  }
}

void handle_faults() {
  pgf_t fault;
  while (sys_get_fault(&fault)) {
    // map in the page or kill the task
    client_t * client = client_find(fault.taskId);
    if (!client) {
      // NOTE: here, there is possibility of a race condition in which some
      // other task will be killed. However, I do not see this as an issue,
      // since no task will be able to execute raw machine code anyway.
      sys_kill(fault.taskId);
      return;
    }
    
  }
}

void handle_client_fault(client_t * cli, pgf_t * fault) {
  // just murder it brutally cause i'm lazy
  sys_kill(cli->pid);
}

