#include "client.h"
#include <stdlib.h>
#include <stdio.h>

void handle_messages(uint64_t fd);
void handle_faults();

void handle_client_fault(client_t * cli, pgf_t * fault);

void main() {
  sys_become_pager();

  sys_sleep(0x100000);
  char * buf = malloc(10);
  printf("buffer is %x\n", buf);
  free(buf);
  printf("free'd buffer\n");
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

