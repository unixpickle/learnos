#include <stdlib.h>
#include <stdio.h>

void command_allocer() {
  printf("allocating 0x10 bytes...\n");
  void * buf = malloc(0x10);
  printf("got result: 0x%x\n", buf);
  free(buf);
  printf("free'd buffer.\n");
  sys_exit();
}

