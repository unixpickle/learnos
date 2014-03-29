#include <string.h>
#include <stdio.h>

static int count = 0;

void command_count() {
  printf("count is 0x%x\n", ++count);
  sys_exit();
}

