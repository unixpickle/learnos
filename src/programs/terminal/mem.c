#include <stdio.h>

void command_memusage() {
  printf("using %x pages.\n", sys_mem_usage());
  sys_exit();
}
