#include <keyedbits/validation.h>
#include <stdio.h>
#include <assert.h>

int main() {
  uint8_t valid[] = {0x81, 0xA6, 0xC6, 0x87, 0x05, 0x25, 0x45, 0x65,
                     0x04, 0x82, 0x83};
  
  uint64_t i;
  for (i = 0; i < sizeof(valid); i++) {
    bool res = kb_validate_header((kb_header_t *)&valid[i]);
    assert(res);
  }
  
  printf("passed!\n");
  
  return 0;
}
