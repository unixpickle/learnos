#include <stdio.h>

void command_memusage() {
  // read away the command message first
  int i = 2;
  while (i) {
    if (!sys_poll()) {
      msg_t msg;
      while (sys_read(0, &msg)) i--;
    }
  }
  printf("using %x pages.\n", sys_mem_usage());
  sys_exit();
}
