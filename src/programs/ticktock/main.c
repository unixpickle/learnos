#include <stdio.h>

int main() {
  uint64_t lastTime = 0;
  while (true) {
    lastTime = sys_get_time();
    // this gets annoying real quick
    printf("It is now 0x%x microseconds after boot.\n", lastTime);
    sys_sleep(0x100000 - (sys_get_time() - lastTime));
  }
  sys_exit();
  return 0;
}

