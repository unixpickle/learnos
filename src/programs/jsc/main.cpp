extern "C" {

#include <stdio.h>

void main() {
  printf("JSC loading...\n");
  sys_sleep(0x100000);
  printf("JSC loaded!\n");
  return;
}

}
