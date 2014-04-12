#include <stdlib.h>
#include <stdio.h>

void command_allocer() {
  void * buf = malloc(0x10);
  printf("allocated 0x10 bytes at 0x%x\n", buf);
  free(buf);
  buf = malloc(0x1000000);
  printf("free'd buffer; allocated 0x1000000 bytes at 0x%x\n", buf);
  sys_exit();
}

