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

void configureAPIC() {
  print64("configuring APIC...\n");
  if (!supportsAPIC()) {
    print64("device does not have an APIC!");
    return;
  }

  print64("APIC base set to: ");
  printHex64((uint64_t)getAPICBase());
  print64("\n");

  unsigned int page = ((uint64_t)getAPICBase() >> 12);
  if (!kernpage_is_mapped(page)) {
    print64("mapping page... ");
    kernpage_create_identity_map(page);
    print64("[OK]\n");
  } else {
    print64("the page is already mapped\n");
  }

  setAPICBase(getAPICBase());
  print64("APIC enabled\n");
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

