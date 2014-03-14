#include <stdio.h>

int main() {
  while (1) {
    uint8_t byte = sys_in(0x64);
    while (byte & 1) {
      uint8_t scan = sys_in(0x60);
      byte = sys_in(0x64);
      printf("got scan %x\n", (uint64_t)scan);
    }
  }
  return 0;
}

