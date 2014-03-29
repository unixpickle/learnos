#include <stdio.h>
#include <string.h>

static void sleep_main();
static void wait_for_msg(msg_t * msg);

void command_sleep() {
  sleep_main();
  sys_exit();
}

static void sleep_main() {
  msg_t msg;
  wait_for_msg(&msg);
  if (msg.len < 6) return;
  char textStr[32];
  memcpy(textStr, msg.message + 6, msg.len - 6 > 31 ? 31 : msg.len - 6);
  textStr[msg.len] = 0;
  uint64_t uSec = xtoi(textStr);
  sys_sleep(uSec);
}

static void wait_for_msg(msg_t * msg) {
  while (1) {
    uint64_t fd = sys_poll();
    if (fd != 0) sys_close(fd);
    while (sys_read(0, msg)) {
      if (msg->type == 1) {
        return;
      }
    }
  }
}

