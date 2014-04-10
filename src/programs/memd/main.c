#include "client.h"
#include <stdlib.h>
#include <stdio.h>

void handle_messages(uint64_t fd);
void handle_faults();

void handle_client_fault(client_t * cli, pgf_t * fault);

void main() {
  sys_become_pager();

  while (1) {
    uint64_t fd = sys_poll();
    printf("[memd]: pull result 0x%x\n", fd);
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
  printf("[memd]: entering get fault loop\n");
  pgf_t fault;
  while (sys_get_fault(&fault)) {
    printf("[memd]: got fault for pid 0x%x\n", fault.taskId);

    // map in the page or kill the task
    client_t * client = client_find(fault.taskId);
    if (!client) {
      sys_kill(fault.taskId);
      sys_shift_fault();
      return;
    }
    handle_client_fault(client, &fault);
    sys_shift_fault();
  }
  printf("done get faults.\n");
}

void handle_client_fault(client_t * cli, pgf_t * fault) {
  // just murder it brutally cause i'm lazy
  sys_kill(cli->pid);
}

