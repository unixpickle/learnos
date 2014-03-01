#include "pit.h"
#include <libkern_base.h>
#include <shared/addresses.h>

void pit_set_divisor(uint16_t div) {
  outb64(0x43, 0x36); // channel 0, lobyte/hibyte, rate generator
  outb64(0x40, (uint8_t)(div & 0xff));
  outb64(0x40, (uint8_t)((div >> 8) & 0xff));
}

void pit_sleep(uint64_t count) {
  uint64_t min = PIT_TICK_COUNT + count;
  while (PIT_TICK_COUNT < min) {
    halt64();
  }
}

