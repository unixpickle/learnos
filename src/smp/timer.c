#include "timer.h"
#include "cpu.h"
#include <interrupts/lapic.h>
#include <shared/addresses.h>

static uint64_t secondLength = 0;

static void save_timer_slice();

void timer_send_eoi() {
  if (lapic_is_in_service(0x20)) {
    lapic_send_eoi();
  }
}

void timer_set_timeout(uint32_t ticks) {
  save_timer_slice();
  if (lapic_is_requested(0x20)) return;
  lapic_timer_set(0x20, ticks);
}

void timer_set_far_timeout() {
  save_timer_slice();
  lapic_timer_set(0xff, 0xffffffff);
}

void timer_cancel_timeout() {
  save_timer_slice();
  lapic_set_register(LAPIC_REG_LVT_TMR, 0x10000);
}

uint64_t timer_get_time() {
  return *((volatile uint64_t *)SYS_TIMESTAMP);
}

uint64_t timer_second_length() {
  if (!secondLength) {
    uint64_t len = lapic_get_bus_speed() * cpu_count();
    // TODO: I like locking things, but maybe I shouldn't
    __asm__("lock xchgq %%rax, %2"
            : "=a" (len)
            : "a" (len), "m" (secondLength));
  }
  return secondLength;
}

static void save_timer_slice() {
  if (lapic_is_in_service(0x20) || lapic_is_requested(0x20)) return;
  // read the last mask
  uint32_t lastLVT = lapic_get_register(LAPIC_REG_LVT_TMR);
  if (lastLVT & 0x10000) return;

  // it was unmasked, so we should see how much time passed
  uint64_t lastInitial = lapic_get_register(LAPIC_REG_TMRINITCNT);
  uint64_t current = lapic_get_register(LAPIC_REG_TMRCURRCNT);

  // I want to use inline assembly here to make sure it's atomic
  __asm__("lock addq %0, (%1)" :
          : "a" (lastInitial - current), "b" (SYS_TIMESTAMP));
}

