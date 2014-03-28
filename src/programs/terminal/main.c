#include <stdio.h>
#include <msgd.h>

static uint64_t keyboard = 0;

int main() {
  char * keyboardName = "keyboard";
  msgd_connect_services(1, (const char **)&keyboardName, &keyboard, 3);
  if (!(keyboard + 1)) {
    printf("[terminal]: error: failed to connect to keyboard\n");
  }
  printf("[terminal]: connected to keyboard.\n");

  while (1) {
    if (sys_poll() == keyboard) {
      msg_t msg;
      while (sys_read(keyboard, &msg)) {
        uint64_t i;
        for (i = 0; i < msg.len; i++) {
          printf("%c", msg.message[i]);
        }
      }
    }
  }
  
  return 0;
}

