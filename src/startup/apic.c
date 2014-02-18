#include <libkern64/stdio.h>
#include <libkern64/basic.h>
#include <shared/types.h>

#define CPUID_FLAG_APIC 0x200
#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800
#define USE_APIC_BASE (unsigned char *)0x201000

static bool supportsAPIC();
static unsigned char * getAPICBase();
static void setAPICBase(unsigned char * base);

static uint32_t readIOAPIC(uint32_t * ptr, uint32_t reg);
static void writeIOAPIC(uint32_t * ptr, uint32_t reg, uint32_t value);

void configureAPIC() {
  print64("configuring APIC...\n");
  if (!supportsAPIC()) {
    print64("device does not have an APIC!");
    return;
  }

  void * addr = getAPICBase();
  print64("APIC base set to: ");
  printHex64((uint64_t)addr);
  print64("\n");

  unsigned int page = ((uint64_t)addr >> 12);
  if (!kernpage_is_mapped(page)) {
    print64("mapping page... ");
    kernpage_create_identity_map(page);
    print64("[OK]\n");
  } else {
    print64("the page is already mapped\n");
  }

  print64("enabling APIC... ");
  setAPICBase(addr);
  print64("APIC address enabled, ");

  // apparently this is the Spourious Interrupt Vector Register
  writeIOAPIC(addr, 0xf0, readIOAPIC(addr, 0xf0) | 0x100);
  print64("done.\n");
}

static bool supportsAPIC() {
  unsigned int ebx, ecx, edx;
  unsigned int eax = cpuid(1, &ebx, &ecx, &edx);
  if (edx & CPUID_FLAG_APIC) return true;
  return false;
}

static unsigned char * getAPICBase() {
  return (unsigned char *)(readMSR(IA32_APIC_BASE_MSR) & 0xffffff100);
}

static void setAPICBase(unsigned char * base) {
  writeMSR(IA32_APIC_BASE_MSR, (uint64_t)base | IA32_APIC_BASE_MSR_ENABLE);
}

static uint32_t readIOAPIC(uint32_t * _ptr, uint32_t reg) {
  volatile uint32_t * ptr = (volatile uint32_t *) _ptr;
  ptr[0] = reg & 0xff;
  return ptr[4];
}

static void writeIOAPIC(uint32_t * _ptr, uint32_t reg, uint32_t value) {
  volatile uint32_t * ptr = (volatile uint32_t *) _ptr;
  ptr[0] = reg & 0xff;
  ptr[4] = value;
}

