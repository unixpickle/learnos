#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

void command_allocer() {
  void * buf = malloc(0x10);
  printf("allocated 0x10 bytes at 0x%x...", buf);
  free(buf);
  printf("freed.\n");
  buf = malloc(0x1000000);
  printf("allocated 0x1000000 bytes at 0x%x...", buf);
  bzero(buf, 0x1000000);
  printf("zero'd.\n");
  sys_exit();
}

