#include "lapic.h"
#include <stdio.h>
#include <libkern_base.h>
#include <kernpage.h>
#include <shared/addresses.h>
#include "acpi.h"
#include "pit.h"

#define LAPIC_BASE_ADDR 0xfee00000
#define LAPIC_TIMER_DIV 0x3 // divide by 16

static void * lapicPtr;
static uint64_t lapicBusSpeed;

void lapic_initialize() {
  if (!acpi_count_lapics()) {
    die("No LAPICs exist");
  }

  if (!lapic_is_x2_available()) {
    if (!lapic_is_available()) {
      die("Local APIC not available");
    }
    // map the memory for the page
    print("mapping Local APIC page... ");
    uint64_t page = (uint64_t)(LAPIC_BASE_ADDR >> 12);
    uint64_t virtualPage = kernpage_last_virtual() + 1;
    if (!kernpage_map(virtualPage, page)) die("failed to map");
    lapicPtr = (void *)(virtualPage << 12);
    print("mapped to 0x");
    printHex((uint64_t)lapicPtr);
    print(" [OK]\n");
  }

  lapic_enable();
  lapic_set_defaults();
  lapic_set_priority(0x0);
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

  lapic_set_register(LAPIC_REG_TMRDIV, LAPIC_TIMER_DIV);
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
  uint64_t flags = msr_read(0x1b) & 0xf00;
  flags |= (uint64_t)lapic_is_x2_available() << 10;
  flags |= 1 << 11;
  msr_write(0x1b, LAPIC_BASE_ADDR | flags);
}

uint32_t lapic_get_id() {
  uint32_t ident = lapic_get_register(LAPIC_REG_APICID);
  if (!lapic_is_x2_available()) return ident >> 0x18;
  return ident;
}

void lapic_clear_errors() {
  lapic_set_register(LAPIC_REG_ESR, 0);
}

uint64_t lapic_calculate_bus_speed() {
  print("calculating LAPIC bus speed...");
  pit_sleep(1);
  lapic_set_register(LAPIC_REG_LVT_TMR, 0xff);
  lapic_set_register(LAPIC_REG_TMRINITCNT, 0xffffffff);
  lapic_set_register(LAPIC_REG_TMRDIV, LAPIC_TIMER_DIV);
  pit_sleep(1);
  uint64_t value = lapic_get_register(LAPIC_REG_TMRCURRCNT);
  lapic_set_register(LAPIC_REG_LVT_TMR, 0x10000);
  print(" value=0x");
  printHex(value);
  print("\n");
  return (lapicBusSpeed = (uint64_t)(0xffffffffL - value) * 100L);
}

uint64_t lapic_get_bus_speed() {
  return lapicBusSpeed;
}

void lapic_send_eoi() {
  lapic_set_register(LAPIC_REG_EOI, 0);
}

bool lapic_is_requested(uint8_t vector) {
  uint64_t regIndex = 0x20 + (vector >> 5);
  uint32_t mask = (1 << (vector & 0x1f));
  return (lapic_get_register(regIndex) & mask) != 0;
}

bool lapic_is_in_service(uint8_t vector) {
  uint64_t regIndex = 0x10 + (vector >> 5);
  uint32_t mask = (1 << (vector & 0x1f));
  return (lapic_get_register(regIndex) & mask) != 0;
}

void lapic_set_priority(uint8_t pri) {
  lapic_set_register(LAPIC_REG_TASKPRIOR, pri);
}

void lapic_set_register(uint16_t reg, uint64_t value) {
  if (lapic_is_x2_available()) {
    msr_write(reg + 0x800, value);
  } else {
    uint64_t base = (uint64_t)lapicPtr;
    if (reg != 0x30) {
      *((volatile uint32_t *)(base + (reg * 0x10))) = (uint32_t)(value & 0xffffffff);
    } else {
      *((volatile uint32_t *)(base + 0x310)) = (uint32_t)(value >> 0x20);
      *((volatile uint32_t *)(base + 0x300)) = (uint32_t)(value & 0xffffffff);
    }
  }
}

uint64_t lapic_get_register(uint16_t reg) {
  if (lapic_is_x2_available()) {
    return msr_read(reg + 0x800);
  } else {
    uint64_t base = (uint64_t)lapicPtr;
    if (reg != 0x30) {
      return (uint64_t)(*((volatile uint32_t *)(base + (reg * 0x10))));
    } else {
      uint32_t lower = *((volatile uint32_t *)(base + 0x300));
      uint32_t upper = *((volatile uint32_t *)(base + 0x310));
      return ((uint64_t)lower) | ((uint64_t)upper << 0x20);
    }
  }
}

void lapic_send_ipi(uint32_t cpu,
                    uint8_t vector,
                    uint8_t mode,
                    uint8_t level,
                    uint8_t trigger) {
  uint64_t value = 0;
  // 1 << 0xb = physical mode
  value = (uint64_t)vector | ((uint64_t)mode << 8) | (0 << 0xb);
  value |= ((uint64_t)level << 0xe) | ((uint64_t)trigger << 0xf);
  if (lapic_is_x2_available()) {
    value |= ((uint64_t)cpu << 0x20);
  } else {
    value |= ((uint64_t)cpu << 0x38);
  }
  lapic_set_register(0x30, value);
}

void lapic_timer_set(uint8_t vector, uint32_t count) {
  uint32_t timerField = vector;// | (2 << 17); // mode is bit 17
  lapic_set_register(LAPIC_REG_LVT_TMR, timerField);
  lapic_set_register(LAPIC_REG_TMRINITCNT, count);
}

