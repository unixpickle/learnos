#include "client.h"
#include <base/system.h>

void handle_messages(uint64_t fd);
void handle_faults();

void main() {
  sys_become_pager();
  while (1) {
    uint64_t fd = sys_poll();
    if (fd + 1) handle_messages(fd);
    handle_faults();
  }
}

void handle_messages(uint64_t fd) {
  
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
      sys_kill(client.taskId);
    }
  }
}

