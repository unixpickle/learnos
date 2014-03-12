#include "libkern32/stdio.h"
#include <shared/addresses.h>
#include <libkern/stdint.h>

static uint32_t roundUpDiv(uint32_t num, uint32_t denom);

/**
 * Creates page tables that map the first contiguous physical memory
 * to virtual memory. This may not include all system memory.
 */
uint32_t basepage_initialize() {
  if ((uint32_t)MBOOT_INFO >= 0x100000) {
    print32("[ERROR] Multiboot info expected in lower 1MB");
    hang32();
  }
  uint32_t flags = MBOOT_INFO[0];
  if (!(flags | 1)) {
    print32("ERROR: No memory information present\n");
    hang32();
  } else if (!(flags | (1 << 6))) {
    print32("ERROR: No mmaps present\n");
    hang32();
  }
  uint32_t kbCount = MBOOT_INFO[2]; // mem_upper field
  // only allocate up to 4GB of virtual memory for now
  if (kbCount > (1 << 22)) {
    kbCount = 1 << 22;
  }

  uint32_t i;
  uint32_t pageCount = (kbCount >> 2) + 0x100;
  uint32_t ptCount = roundUpDiv(pageCount, 0x200);
  uint32_t pdtCount = roundUpDiv(ptCount, 0x200);
  uint32_t pdptCount = roundUpDiv(pdtCount, 0x200);

  print32("Initially creating ");
  printHex32(pageCount);
  print32(" pages, ");
  printHex32(ptCount);
  print32(" PTs, ");
  printHex32(pdtCount);
  print32(" PDTs, ");
  printHex32(pdptCount);
  print32(" PDPTs.\n");

  uint32_t totalCount = ptCount + pdtCount + pdptCount + 1;
  LAST_PAGE = (uint64_t)(totalCount - 1 + (PML4_START >> 12));
  LAST_VPAGE = (uint64_t)(pageCount - 1);
  if (LAST_PAGE > LAST_VPAGE) {
    print32("[ERROR]: trying to use more space with page tables than vmem\n");
    hang32();
  }

  // zero the memory for our descriptors
  unsigned char * dataPtr = (unsigned char *)PML4_START;
  for (i = 0; i < totalCount * 0x1000; i++) {
    dataPtr[i] = 0;
  }

  // create the PT entries
  uint32_t ptOffset = (pdtCount + pdptCount + 1) * 0x1000;
  uint64_t * ptEntries = (uint64_t *)(PML4_START + ptOffset);
  uint64_t addr = 3;
  for (i = 0; i < pageCount; i++) {
    ptEntries[i] = addr;
    addr += 0x1000;
  }

  // create the PDT entries
  uint32_t pdtOffset = (pdptCount + 1) * 0x1000;
  uint64_t * pdtEntries = (uint64_t *)(PML4_START + pdtOffset);
  addr = PML4_START + ptOffset + 3;
  for (i = 0; i < ptCount; i++) {
    pdtEntries[i] = addr;
    addr += 0x1000;
  }

  // create the PDPT entries
  uint32_t pdptOffset = 0x1000;
  uint64_t * pdptEntries = (uint64_t *)(PML4_START + pdptOffset);
  addr = PML4_START + pdtOffset + 3;
  for (i = 0; i < pdtCount; i++) {
    pdptEntries[i] = addr;
    addr += 0x1000;
  }

  // and lastly, create the PML4 entries
  uint64_t * pml4Entries = (uint64_t *)PML4_START;
  addr = PML4_START + pdptOffset + 3;
  for (i = 0; i < pdptCount; i++) {
    pml4Entries[i] = addr;
    addr += 0x1000;
  }

  return totalCount * 0x1000;
}

static uint32_t roundUpDiv(uint32_t num, uint32_t denom) {
  uint32_t result = num / denom;
  if (num % denom) return result + 1;
  return result;
}

