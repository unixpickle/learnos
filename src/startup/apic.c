#include <libkern64/stdio.h>
#include <libkern64/basic.h>
#include <shared/types.h>
#include <shared/addresses.h>

#define CPUID_FLAG_APIC 0x200
#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_ENABLE 0x800

static bool supportsAPIC();
static unsigned char * getAPICBase();
static void setAPICBase(unsigned char * base);

static uint32_t readIOAPIC(uint32_t reg);
static void writeIOAPIC(uint32_t reg, uint32_t value);

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

  print64("mapping page... ");
  uint64_t page = (uint64_t)addr >> 12;
  uint64_t virtualPage = kernpage_next_virtual();
  if (!kernpage_map(virtualPage, page)) {
    print64("[FAIL]\n");
    hang64();
  } else {
    print64("[OK]\n");
  }
  uint64_t virtualOffset = (virtualPage << 12) + ((uint64_t)addr & 0xfff);
  *((void **)APIC_PTR) = (void *)virtualOffset;

  print64("enabling APIC... ");
  setAPICBase(addr);
  print64("APIC address enabled, ");

  // apparently this is the Spourious Interrupt Vector Register
  writeIOAPIC(0xf0, readIOAPIC(0xf0) | 0x100);
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

static uint32_t readIOAPIC(uint32_t reg) {
  volatile uint32_t * ptr = *((volatile uint32_t **)APIC_PTR);
  ptr[0] = reg & 0xff;
  return ptr[4];
}

static void writeIOAPIC(uint32_t reg, uint32_t value) {
  volatile uint32_t * ptr = *((volatile uint32_t **)APIC_PTR);
  ptr[0] = reg & 0xff;
  ptr[4] = value;
}

