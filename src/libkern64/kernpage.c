#include <shared/types.h>
#include <shared/addresses.h>

#define PAGE_MASK 0xfffffffffffff000L

bool kernpage_is_mapped(uint64_t page) {
  // we have one PML4, many PDPT's, many PDT's, and many PT's
  // our page tables will begin to become fragmented, but that's okay
  uint64_t indexInPT = page % 0x200;
  uint64_t indexInPDT = (page >> 12) % 0x200;
  uint64_t indexInPDPT = (page >> 24) % 0x200;
  uint64_t indexInPML4 = (page >> 32) % 0x200;

  // check the entry in the PML4
  uint64_t * tablePtr = (uint64_t *)PML4_START;
  uint64_t value = tablePtr[indexInPML4];
  if (!(value & 3)) return false;

  // check the entry in the PDPT
  tablePtr = (uint64_t *)(value & PAGE_MASK);
  value = tablePtr[indexInPDPT];
  if (!(value & 3)) return false;

  // check the entry in the PDT
  tablePtr = (uint64_t *)(value & PAGE_MASK);
  value = tablePtr[indexInPDT];
  if (!(value & 3)) return false;

  // finally, check the PT entry
  tablePtr = (uint64_t *)(value & PAGE_MASK);
  if (!(tablePtr[indexInPT] & 3)) return false;
  return true;
}

void kernpage_create_identity_map(uint64_t page) {
  unsigned int pageBytes = *((const unsigned int *)END_PAGE_INFO);
  uint64_t * tablesEnd = (uint64_t *)(PML4_START + (uint64_t)pageBytes);
  int i, j;

  uint64_t indexInPT = page % 0x200;
  uint64_t indexInPDT = (page >> 12) % 0x200;
  uint64_t indexInPDPT = (page >> 24) % 0x200;
  uint64_t indexInPML4 = (page >> 32) % 0x200;

  // recursively create pages at the end of our paging structure
  uint64_t indices[4] = {indexInPML4, indexInPDPT, indexInPDT, indexInPT};
  uint64_t * tablePtr = (uint64_t *)PML4_START;
  for (i = 0; i < 4; i++) {
    uint64_t value = tablePtr[indices[i]];
    if (!(value & 3)) {
      if (i == 3) {
        // put in the address of the actual page
        tablePtr[indices[i]] = (page << 12) | 0x3;
        break;
      }
      // create a new sub-table
      for (j = 0; j < 0x200; j++) {
        tablesEnd[j] = 0;
      }
      value = ((uint64_t)tablesEnd) | 0x3;
      tablePtr[indices[i]] = value;
      tablesEnd += 0x200;
      pageBytes += 0x1000;
    }
  }
  *((unsigned int *)END_PAGE_INFO) = pageBytes;
}

