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
    // map the memory for the page
    print64("mapping Local APIC page... ");
    uint64_t page = (uint64_t)(LAPIC_BASE_ADDR >> 12);
    uint64_t virtualPage = kernpage_next_virtual();
    if (!kernpage_map(virtualPage, page)) die("failed to map");
    LAPIC_PTR = (void *)(virtualPage << 12);
    print64("mapped to 0x");
    printHex64((uint64_t)LAPIC_PTR);
    print64(" [OK]\n");
  }

  lapic_set_defaults();
  lapic_set_priority(0x0);
  lapic_enable();
}

void lapic_set_defaults() {
  lapic_set_register(LAPIC_REG_TASKPRIOR, 0x20);
  lapic_set_register(LAPIC_REG_LVT_TMR, 0x10000);
  lapic_set_register(LAPIC_REG_LVT_PERF, 0x10000);
  lapic_set_register(LAPIC_REG_LVT_LINT0, 0x8700);
  lapic_set_register(LAPIC_REG_LVT_LINT1, 0x400);
  lapic_set_register(LAPIC_REG_LVT_ERR, 0x10000);
  lapic_set_register(LAPIC_REG_SPURIOUS, 0x1ff);

  // reset might have shut them off
  lapic_set_register(LAPIC_REG_LVT_LINT0, 0x8700);
  lapic_set_register(LAPIC_REG_LVT_LINT1, 0x400);
  print64("SIV is ");
  printHex64(lapic_get_register(LAPIC_REG_SPURIOUS));
  print64("\n");
}

bool lapic_is_x2_available() {
  return false; // for now, x2APIC will not be supported :\
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
  uint64_t flags = msr_read(0x1b) & 0xf00;
  flags |= (uint64_t)lapic_is_x2_available() << 10;
  flags |= 1 << 11;
  msr_write(0x1b, LAPIC_BASE_ADDR | flags);
}

void lapic_send_eoi() {
  lapic_set_register(LAPIC_REG_EOI, 0);
}

void lapic_set_priority(uint8_t pri) {
  lapic_set_register(LAPIC_REG_TASKPRIOR, pri);
}

void lapic_set_register(uint16_t reg, uint64_t value) {
  if (lapic_is_x2_available()) {
    msr_write(reg + 0x800, value);
  } else {
    uint64_t base = (uint64_t)LAPIC_PTR;
    if (reg != 0x30) {
      *((volatile uint32_t *)(base + (reg * 0x10))) = (uint32_t)(value & 0xffffffff);
    } else {
      *((volatile uint32_t *)(base + 0x300)) = (uint32_t)(value & 0xffffffff);
      *((volatile uint32_t *)(base + 0x310)) = (uint32_t)(value >> 0x20);
    }
  }
}

uint64_t lapic_get_register(uint16_t reg) {
  if (lapic_is_x2_available()) {
    return msr_read(reg + 0x800);
  } else {
    uint64_t base = (uint64_t)LAPIC_PTR;
    if (reg != 0x30) {
      return (uint64_t)(*((volatile uint32_t *)(base + (reg * 0x10))));
    } else {
      uint32_t lower = *((volatile uint32_t *)(base + 0x300));
      uint32_t upper = *((volatile uint32_t *)(base + 0x310));
      return ((uint64_t)lower) | ((uint64_t)upper << 0x20);
    }
  }
}

