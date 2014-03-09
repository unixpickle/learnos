#include "gdt.h"
#include <shared/addresses.h>

extern void GDT64_pointer();

void gdt_initialize() {
  uint16_t * gdtPtr = (uint16_t *)GDT64_pointer;
  uint8_t * dataPtr = *((uint8_t **)(gdtPtr + 1));
  uint8_t * tableDest = (uint8_t *)DYNAMIC_GDT;
  uint16_t len = *gdtPtr;
  uint32_t i;
  for (i = 0; i <= len; i++) {
    tableDest[i] = dataPtr[i];
  }
  uint16_t * newLenPtr = (uint16_t *)GDT64_PTR;
  (*newLenPtr) = len;
  uint64_t * newPtr = (uint64_t *)(GDT64_PTR + 2);
  (*newPtr) = (uint64_t)DYNAMIC_GDT;
}

uint64_t gdt_get_size() {
  return (1 + *((uint16_t *)GDT64_PTR));
}

tss_t * gdt_add_tss() {
  uint64_t size = gdt_get_size();
  uint64_t index = (size - 0x10) / 0x10;
  uint64_t ptr = TSS_ENTRIES + (index * 0x68);

  // zero out the TSS
  uint32_t * tss = (uint32_t *)ptr;
  int i;
  for (i = 0; i < 0x1a; i++) tss[i] = 0;
  tss[0x19] = 0xffff0000; // IOPL points *way* in the distance

  tss_descriptor_t desc;
  desc.limit_0 = 0x67;
  desc.limit_16 = 0;
  desc.base_0 = (uint16_t)(ptr & 0xffff);
  desc.base_16 = (uint8_t)((ptr >> 16) & 0xff);
  desc.base_24 = (uint8_t)((ptr >> 24) & 0xff);
  desc.base_32 = (uint32_t)(ptr >> 32);
  desc.res0 = 0;
  desc.res1 = 0;
  desc.res2 = 0;
  desc.granularity = 0;
  desc.present = 1;
  desc.available = 1;
  desc.type = 9; // 64-bit TSS
  uint32_t * dest = (uint32_t *)(DYNAMIC_GDT + size);
  uint32_t * source = (uint32_t *)&desc;
  for (i = 0; i < 4; i++) {
    dest[i] = source[i];
  }
  (*((uint16_t *)GDT64_PTR)) += sizeof(desc);

  return (tss_t *)tss;
}

