#include "lapic.h"
#include <libkern64/stdio.h>
#include <shared/addresses.h>
#include "acpi.h"

#define LAPIC_BASE_ADDR 0xfee00000

void lapic_initialize() {
  if (!acpi_count_lapics()) {
    die("No LAPICs exist");
  }

  if (!lapic_is_x2_available()) {
    if (!lapic_is_available()) {
      die("Local APIC not available");
    }
  }

  // map the memory for the page
  print64("mapping Local APIC page... ");
  uint64_t page = (uint64_t)(LAPIC_BASE_ADDR >> 12);
  uint64_t virtualPage = kernpage_next_virtual();
  if (!kernpage_map(virtualPage, page)) die("failed to map");
  LAPIC_PTR = (void *)(virtualPage << 12);
  print64("mapped to 0x");
  printHex64((uint64_t)LAPIC_PTR);
  print64(" [OK]\n");

  lapic_set_defaults();
  lapic_set_priority(0x0);
  lapic_enable();
}

void lapic_set_defaults() {
  uint64_t base = (uint64_t)LAPIC_PTR;
  *((volatile uint32_t *)(base + LAPIC_REG_TASKPRIOR)) = 0x20; // stop softint deliv
  *((volatile uint32_t *)(base + LAPIC_REG_LVT_TMR)) = 0x10000; // disable timer
  *((volatile uint32_t *)(base + LAPIC_REG_LVT_PERF)) = 0x10000; // disable perf ints
  *((volatile uint32_t *)(base + LAPIC_REG_LVT_LINT0)) = 0x8700; // normal ext ints
  *((volatile uint32_t *)(base + LAPIC_REG_LVT_LINT1)) = 0x40; // normal NMI proc
  *((volatile uint32_t *)(base + LAPIC_REG_LVT_ERR)) = 0x10000; // disable error ints
  *((volatile uint32_t *)(base + LAPIC_REG_SPURIOUS)) = 0x10f; // spurious vec=15
}

bool lapic_is_x2_available() {
  // ebx, ecx, edx
  uint32_t ebx, ecx, edx;
  cpuid(1, &ebx, &ecx, &edx);
  return (ecx & (1 << 21)) != 0;
}

bool lapic_is_available() {
  // ebx, ecx, edx
  uint32_t ebx, ecx, edx;
  cpuid(1, &ebx, &ecx, &edx);
  return (edx & (1 << 9)) != 0;
}

void lapic_enable() {
  // set enable x2 and regular
  uint64_t flags = msr_read(0x1b) & (0xf00);
  flags |= (uint64_t)lapic_is_x2_available() << 10;
  flags |= 1 << 11;
  msr_write(0x1b, LAPIC_BASE_ADDR | flags);
}

void lapic_send_eoi() {
  volatile uint32_t * addr = (uint32_t *)((uint64_t)LAPIC_PTR + LAPIC_REG_EOI);
  addr[0] = 0;
}

void lapic_set_priority(uint8_t pri) {
  volatile uint32_t * addr = (uint32_t *)((uint64_t)LAPIC_PTR + LAPIC_REG_TASKPRIOR);
  addr[0] = (uint32_t)pri;
}

