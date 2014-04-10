#include <stdio.h>

void command_fault() {
  printf("about to attempt fault...\n");
  uint8_t * values = (uint8_t *)0x0;
  values[0] = 1;
  printf("got away with it!\n");
  sys_exit();
}
