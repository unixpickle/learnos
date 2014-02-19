#include "kernpage.h"
#include <shared/addresses.h>

typedef struct {
  uint32_t size;
  uint64_t base_addr;
  uint64_t length;
  uint32_t type;
} __attribute__((packed)) mmap_info;

typedef struct {
  uint64_t start;
  uint64_t length;
} __attribute__((packed)) kernpage_info;

static void _kernpage_initialize_mapping();
static void _kernpage_page_physical();

void kernpage_initialize() {
  KERNPAGE_ENABLED = 1;
  _kernpage_initialize_mapping();
  _kernpage_page_physical();
  print64("last pages: virtual=0x");
  printHex64((uint64_t)LAST_VPAGE);
  print64(" physical=0x");
  printHex64((uint64_t)LAST_PAGE);
  print64("\n");
  hang64();
}

void kernpage_lockdown() {
  KERNPAGE_ENABLED = 0;
}

uint64_t kernpage_next_physical() {
  uint64_t page = LAST_PAGE;
  int i;
  const kernpage_info * maps = (const kernpage_info *)PHYSICAL_MAP_ADDR;
  for (i = 0; i < PHYSICAL_MAP_COUNT; i++) {
    kernpage_info map = maps[i];
    if (map.start + map.length > page && map.start <= page) {
      if (page < map.start + map.length - 1) {
        return page + 1;
      } else if (i == PHYSICAL_MAP_COUNT - 1) {
        return 0;
      } else {
        return maps[i + 1].start;
      }
    }
  }
  return 0;
}

uint64_t kernpage_calculate_virtual(uint64_t physicalPage) {
  int i;
  const kernpage_info * maps = (const kernpage_info *)PHYSICAL_MAP_ADDR;
  uint64_t virtual = 0;
  for (i = 0; i < PHYSICAL_MAP_COUNT; i++) {
    if (maps[i].start > physicalPage) return 0; // we've gone too far
    if (maps[i].start <= physicalPage && maps[i].start + maps[i].length > physicalPage) {
      return virtual + physicalPage - maps[i].start;
    }
    virtual += maps[i].length;
  }
  return 0;
}

uint64_t kernpage_calculate_physical(uint64_t virtualPage) {
  int i;
  const kernpage_info * maps = (const kernpage_info *)PHYSICAL_MAP_ADDR;
  for (i = 0; i < PHYSICAL_MAP_COUNT; i++) {
    if (maps[i].start + maps[i].length > virtualPage) {
      return maps[i].start + virtualPage;
    }
    virtualPage -= maps[i].length;
  }
  return 0;
}

uint64_t kernpage_allocate_physical() {
  uint64_t next = kernpage_next_physical();
  if (!next) return 0;
  return (LAST_PAGE = next);
}

uint64_t kernpage_allocate_contiguous(uint64_t count) {
  uint64_t page = LAST_PAGE;
  int i;
  const kernpage_info * maps = (const kernpage_info *)PHYSICAL_MAP_ADDR;
  uint64_t result = 0;
  for (i = 0; i < PHYSICAL_MAP_COUNT; i++) {
    kernpage_info map = maps[i];
    if (map.start <= page) {
      if (page < map.start + map.length - count) {
        result = page + 1;
        break;
      } else if (i == PHYSICAL_MAP_COUNT - 1) {
        return 0;
      }
    }
  }
  if (result == 0) return result;
  LAST_PAGE = result + count - 1;
  return result;
}

bool kernpage_is_mapped(uint64_t page) {
  if (page > LAST_VPAGE) return false;
  int i;

  // check if it's set in the page table
  uint64_t indexInPT = page % 0x200;
  uint64_t indexInPDT = (page >> 9) % 0x200;
  uint64_t indexInPDPT = (page >> 18) % 0x200;
  uint64_t indexInPML4 = (page >> 27) % 0x200;
  uint64_t indices[4] = {indexInPML4, indexInPDPT, indexInPDT, indexInPT};
  uint64_t * tablePtr = (uint64_t *)PML4_START;
  for (i = 0; i < 4; i++) {
    uint64_t value = tablePtr[indices[i]];
    if (!(value & 0x03)) {
      return false;
    }
    uint64_t physPage = value >> 12;
    uint64_t virPage = kernpage_calculate_virtual(physPage);
    tablePtr = (uint64_t *)(virPage << 12);
  }
  return true;
}

bool kernpage_map(uint64_t virtualPage, uint64_t physicalPage) {
  // check if it's set in the page table
  int i;
  uint64_t indexInPT = virtualPage % 0x200;
  uint64_t indexInPDT = (virtualPage >> 9) % 0x200;
  uint64_t indexInPDPT = (virtualPage >> 18) % 0x200;
  uint64_t indexInPML4 = (virtualPage >> 27) % 0x200;
  uint64_t indices[4] = {indexInPML4, indexInPDPT, indexInPDT, indexInPT};
  volatile uint64_t * tablePtr = (uint64_t *)PML4_START;
  for (i = 0; i < 3; i++) {
    uint64_t value = tablePtr[indices[i]];
    if (!(value & 0x03)) {
      // create a subtable
      uint64_t newPage = kernpage_allocate_physical();
      if (!newPage) return false;
      uint64_t newVirPage = kernpage_calculate_virtual(newPage);
      if (!newVirPage) return false;
      volatile uint64_t * newData = (uint64_t *)(newVirPage << 12);
      int j;
      for (j = 0; j < 0x200; j++) newData[j] = 0;
      tablePtr[indices[i]] = (newPage << 12) | 3;
      tablePtr = newData;
    } else {
      uint64_t physPage = value >> 12;
      uint64_t virPage = kernpage_calculate_virtual(physPage);
      if (!virPage) return false;
      tablePtr = (uint64_t *)(virPage << 12);
    }
  }
  tablePtr[indices[3]] = (physicalPage << 12) | 3;
  if (virtualPage > LAST_VPAGE) LAST_VPAGE = virtualPage;
  return true;
}

uint64_t kernpage_next_virtual() {
  return LAST_VPAGE + 1;
}

static void _kernpage_initialize_mapping() {
  uint32_t mmapLength = MBOOT_INFO[11];
  uint64_t mmapAddr = (uint64_t)MBOOT_INFO[12];

  // By this point, startup/page_init.c has already given us access
  // to enough physical memory that we should definitely be able
  // to access the mmap structure.
  const mmap_info * mmap = NULL;
  const mmap_info * next = (const mmap_info *)((uint64_t)mmapAddr);
  volatile kernpage_info * destMap = (kernpage_info *)PHYSICAL_MAP_ADDR;

  int count = 0, i;
  while (mmapLength > 0 && count < 255) {
    // iterating maps
    mmap = next;
    next = (const mmap_info *)((const char *)mmap + mmap->size + 4);
    mmapLength -= mmap->size + 4;
    if (mmap->type != 1) continue;

    // This list will not be perfectly 1-to-1 because we identity map
    // some lower memory from page_init.c, so here we need to make sure
    // we respect that
    if (mmap->base_addr + mmap->length <= 0x100000) continue;

    // make sure that the memory is page aligned
    uint64_t start = mmap->base_addr;
    uint64_t len = mmap->length;
    if (start & 0xfff) {
      // push up the memory to the next
      uint64_t shard = 0x1000 - (start & 0xfff);
      if (len <= shard) continue;
      start += shard;
      len -= shard;
    }
    len >>= 12;
    start >>= 12;

    if (start <= 0x100) {
      // the first physical segment must map the lower 1MB at least
      len += start;
      start = 0;
    }
    kernpage_info info = {start, len};
    print64("adding info {");
    printHex64(info.start);
    print64(", ");
    printHex64(info.length);
    print64("}\n");

    // insert the kernpage_info where it fits
    int insIndex = count;
    for (i = 0; i < count; i++) {
      if (destMap[i].start > start) {
        insIndex = i;
        break;
      }
    }
    for (i = count; i > insIndex; i--) {
      destMap[i] = destMap[i - 1];
    }
    destMap[insIndex] = info;
    count++;
  }
  if (count < 1) {
    print64("[ERROR]: no immediate upper memory\n");
    hang64();
  }
  PHYSICAL_MAP_COUNT = (uint8_t)count;
  print64("There are ");
  printHex64(count);
  print64(" memory maps: ");
  for (i = 0; i < count; i++) {
    if (i != 0) print64(", ");
    print64("0x");
    printHex64(destMap[i].start);
    print64(":");
    printHex64(destMap[i].length);
  }
  print64("\n");
}

static void _kernpage_page_physical() {
  print64("expanding physical map...\n");
  // map pages
  int i;
  uint64_t j;
  const kernpage_info * maps = (const kernpage_info *)PHYSICAL_MAP_ADDR;
  uint64_t virtual = 0, created = 0;
  print64("last page (initial) = ");
  printHex64(LAST_PAGE);
  print64(", vpage = ");
  printHex64(LAST_VPAGE);
  print64("\n");
  for (i = 0; i < PHYSICAL_MAP_COUNT; i++) {
    for (j = maps[i].start; j < maps[i].length + maps[i].start; j++) {
      if (!kernpage_is_mapped(virtual)) {
        kernpage_map(virtual, j);
        created++;
      }
      virtual++;
    }
  }
  print64("mapped 0x");
  printHex64(created);
  print64(" new pages to virtual memory\n");
}

