#include <libkern32/stdio.h>

#define PML4_START 0x300000

static unsigned int roundUpDiv(unsigned int num, unsigned int denom);

typedef unsigned long long uint64_t;

/**
 * Creates page tables that map all physical memory into
 * the same virtual addresses.
 * @param the KBs used in upper memory, taken from multiboot
 * @return The pointer to the end of reserved memory
 */
void * configurePages(unsigned int kbCount) {
  unsigned int i, j;
  unsigned int pageCount = (kbCount >> 2) + 0x100;
  unsigned int ptCount = roundUpDiv(pageCount, 0x200);
  unsigned int pdtCount = roundUpDiv(pageCount, 0x40000);
  unsigned int pdptCount = roundUpDiv(pageCount, 0x8000000);

  print32("there are ");
  printHex32(pageCount);
  print32(" pages, ");
  printHex32(ptCount);
  print32(" page tables, ");
  printHex32(pdtCount);
  print32(" pdt, ");
  printHex32(pdptCount);
  print32(" pdpt.\n");

  // zero the memory for our descriptors
  unsigned int totalCount = ptCount + pdtCount + pdptCount + 1;
  unsigned char * dataPtr = (unsigned char *)PML4_START;
  for (i = 0; i < totalCount * 0x1000; i++) {
    dataPtr[i] = 0;
  }

  // create the PT entries
  unsigned int ptOffset = (pdtCount + pdptCount + 1) * 0x1000;
  uint64_t * ptEntries = (uint64_t *)(PML4_START + ptOffset);
  uint64_t addr = 3;
  for (i = 0; i < pageCount; i++) {
    ptEntries[i] = addr;
    addr += 0x1000;
  }

  // create the PDT entries
  unsigned int pdtOffset = (pdptCount + 1) * 0x1000;
  uint64_t * pdtEntries = (uint64_t *)(PML4_START + pdtOffset);
  addr = PML4_START + ptOffset + 3;
  for (i = 0; i < ptCount; i++) {
    pdtEntries[i] = addr;
    addr += 0x1000;
  }

  // create the PDPT entries
  unsigned int pdptOffset = 0x1000;
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

  return ptEntries + (ptCount * 0x200);
}

static unsigned int roundUpDiv(unsigned int num, unsigned int denom) {
  unsigned int result = num / denom;
  if (num % denom) return result + 1;
  return result;
}

