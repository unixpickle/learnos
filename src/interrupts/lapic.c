#include "lapic.h"
#include <libkern64/stdio.h>
#include <shared/addresses.h>
#include "acpi.h"

#define LAPIC_BASE_ADDR 0xfee00000

void lapic_initialize() {
  if (!acpi_count_lapics()) {
    die("No LAPICs exist");
  }
  // TODO: don't depend on this
  if (!lapic_is_available()) {
    die("x2APIC not available");
  }

  // map the memory for the page
  print64("mapping I/O APIC page... ");
  uint64_t page = (uint64_t)(LAPIC_BASE_ADDR >> 12);
  uint64_t virtualPage = kernpage_next_virtual();
  if (!kernpage_map(virtualPage, page)) die("failed to map");
  LAPIC_PTR = (void *)(virtualPage << 12);
  print64("mapped to 0x");
  printHex64((uint64_t)LAPIC_PTR);
  print64(" [OK]\n");

  lapic_enable();
}

bool lapic_is_available() {
  // ebx, ecx, edx
  uint32_t ebx, ecx, edx;
  cpuid(1, &ebx, &ecx, &edx);
  print64("  ecx: ");
  printHex64(ecx);
  print64("\n");
  return (ecx & (1 << 21)) != 0;
}

void lapic_enable() {
  // set enable x2 and regular
  uint64_t flags = msr_read(0x1b) & (0b11111 << 8);
  msr_write(0x1b, LAPIC_BASE_ADDR | flags | (3 << 10));
}

