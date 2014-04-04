#include <stdio.h>
#include <string.h>

void command_echo() {
  // the first message we get is the command they typed
  msg_t msg;
  while (1) {
    uint64_t fd = sys_poll();
    if (fd != 0) sys_close(fd);
    while (sys_read(0, &msg)) {
      if (msg.type == 1) {
        if (msg.len > 5) {
          sys_write(0, msg.message + 5, msg.len - 5);
        }
        char terminator[2] = "\n";
        sys_write(0, terminator, 1);
        sys_exit();
      }
    }
  }
}

