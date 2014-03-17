#include <stdio.h>

static uint8_t get_next_byte();
static uint32_t scancode_in();
static bool byte_out(uint8_t byte);

int main() {
  sys_print("loading keyboard driver.\n");

  // set scan code set 2
  if (byte_out(0xf0)) {
    printf("writing out scan code set\n");
    byte_out(1);
  }

  while (1) {
    uint32_t input = scancode_in();
    if (input == 0x1c) {
      printf("A");
    } else printf("unknown scancode %x\n", input);
  }

  return 0;
}

static uint8_t get_next_byte() {
  while (1) {
    uint8_t byte = sys_in(0x64);
    if (byte & 1) {
      return sys_in(0x60);
    }
    while (!(sys_getint() & 2));
  }
}

static uint32_t scancode_in() {
  uint8_t code = get_next_byte();
  if (code == 0xe0) {
    uint8_t nextCode = get_next_byte();
    if (nextCode == 0xf0) {
      uint8_t nextNext = get_next_byte();
      uint8_t nextNextNext = get_next_byte();
      return code | (nextCode << 8) | (nextNext << 16) | (nextNextNext << 24);
    } else {
      return code | (nextCode << 8) | (get_next_byte() << 16);
    }
  } else if (code == 0xf0) {
    return code | (get_next_byte() << 8);
  } else {
    return code;
  }
}

static bool byte_out(uint8_t byte) {
  // wait up to 100 ms to write a byte
  uint64_t startTime = sys_get_time();
  bool ready = false;
  while (sys_get_time() < startTime + 100) {
    if (sys_in(0x64) & 1) {
      ready = true;
      break;
    }
  }
  if (!ready) return false;
  sys_out(0x60, byte);
  return true;
}

