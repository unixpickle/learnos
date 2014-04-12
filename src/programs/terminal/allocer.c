#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

void command_allocer() {
  void * buf = malloc(0x10);
  printf("allocated 0x10 bytes at 0x%x...", buf);
  free(buf);
  printf("freed.\n");
  buf = malloc(0x2000000);
  printf("allocated 0x2000000 bytes at 0x%x...", buf);
  uint64_t now = sys_get_time();
  bzero(buf, 0x2000000);
  printf("zero'd in %x usecs\n", sys_get_time() - now);
  sys_exit();
}

