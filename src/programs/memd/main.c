#include "client.h"
#include <base/system.h>

void handle_messages(uint64_t fd);

void main() {
  while (1) {
    uint64_t fd = sys_poll();
    if (fd + 1) handle_messages(fd);
    // here, read for page fault notificaitons
  }
}

void handle_messages(uint64_t fd) {
  
}

