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
    if (fd + 1) handle_messages(fd);
    handle_faults();
  }
}

void handle_messages(uint64_t fd) {
  client_t * cli = client_get(fd);

  msg_t msg;
  while (sys_read(fd, &msg)) {
    if (msg.type == 2) {
      client_delete(cli);
      sys_close(fd);
      return;
    }
    if (msg.type == 0) continue;

    kb_buff_t kb;
    kb_buff_initialize_decode(&kb, (void *)msg.message, msg.len);
    
    // TODO: parse the message here
    printf("got client message, but who *really* cares, man\n");
  }
}

void handle_faults() {
  pgf_t fault;
  while (sys_get_fault(&fault)) {
    // map in the page or kill the task
    client_t * client = client_find(fault.taskId);
    if (!client) {
      sys_mem_fault(fault.taskId);
      sys_shift_fault();
      continue;
    }
    handle_client_fault(client, &fault);
    sys_shift_fault();
  }
  printf("done get faults.\n");
}

void handle_client_fault(client_t * cli, pgf_t * fault) {
  printf("client faulted because i'm a murderer.\n");
  // just murder it brutally cause i'm lazy
  sys_mem_fault(cli->pid);
}

