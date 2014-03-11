#include <stdio.h>
#include <stdint.h>
#include <interrupts/lapic.h>

void ensure_critical() {
  uint64_t flags;
  __asm__ ("pushfq\npop %0" : "=r" (flags));
  if (flags & (1 << 9)) {
    die("ZONE IS NOT CRITICAL\n");
  }
}

void print_stack() {
  uint64_t rsp;
  __asm__ ("mov %%rsp, %0" : "=r" (rsp));
  print("stack is ");
  printHex(rsp);
  print(" for CPU ");
  printHex(lapic_get_id());
  print("\n");
}

