#include <anmem/config.h>
#include <analloc.h>
#include <anpages.h>

#define PAGE_INVAL 0xffffffffffffffffL

static bool _create_controllable(anmem_config_t * config,
                                 anmem_t * mem,
                                 uint64_t pagesExp,
                                 uint64_t pageSkip);
static bool _add_allocator(anmem_t * mem,
                           uint64_t start,
                           uint64_t len,
                           uint64_t type);
static uint64_t _sum_regions(anmem_config_t * config);
static uint64_t _next_analloc(anmem_t * mem, uint64_t start, uint64_t * len);
static bool _region_is_taken(anmem_t * mem, uint64_t start, uint64_t len);

bool anmem_configure(anmem_config_t * config,
                     anmem_t * mem,
                     uint64_t maxControllable,
                     uint64_t pageSkip) {
  if (!mem->maximum) return false;
  bool result = _create_controllable(config, mem, maxControllable, pageSkip);
  
  uint64_t allocs = 0;
  uint64_t sum = _sum_regions(config);
  bool hadToCut = false;
  
  // NOTE: anpages cannot be used on single-page regions.
  
  do {
    // calculate the number of anpages_t we will need
    uint64_t page = pageSkip;
    while (page + 1 < sum) {
      uint64_t len;
      uint64_t next = _next_analloc(mem, page, &len);
      if (!(next + 1)) {
        allocs++;
        break;
      }
      if (next - page > 1) allocs++;
      page = next + len;
    }
    
    if (allocs + mem->count > mem->maximum) {
      mem->count--;
      hadToCut = true;
    }
  } while (allocs + mem->count > mem->maximum);
  
  // go through and create the allocators
  uint64_t startPage = pageSkip;
  
  while (startPage + 1 < sum) {
    uint64_t len;
    uint64_t next = _next_analloc(mem, startPage, &len);
    if (!(next + 1)) {
      // fill up the rest of the memory
      _add_allocator(mem, startPage, sum - startPage, 0);
      break;
    }
    
    if (next - startPage > 1) {
      _add_allocator(mem, startPage, next - startPage, 0);
    }
    
    startPage = next + len;
  }
  
  return result && !hadToCut;
}

bool anmem_init_structures(anmem_t * mem) {
  uint64_t i;
  for (i = 0; i < mem->count; i++) {
    uint64_t type = mem->allocators[i].type;
    uint64_t start = mem->allocators[i].start;
    uint64_t len = mem->allocators[i].len;
    if (type == 0) { // anpages
      anpages_t pages = &mem->allocators[i].anpagesRoot;
      if (!anpages_initialize(pages, start, len)) {
        return false;
      }
    } else if (type == 1) { // analloc
      void * buffer = (void *)(start << 12);
      analloc_t alloc = &mem->allocators[i].anallocRoot;
      if (!analloc_with_chunk(alloc, buffer, len << 12, 0, 0x1000)) {
        return false;
      }
    }
  }
  return true;
}

uint64_t anmem_analloc_count(anmem_t * mem) {
  // count the size used by anallocators
  uint64_t i, pages = 0;
  for (i = 0; i < mem->count; i++) {
    if (mem->allocators[i].type == 1) {
      pages += mem->allocators[i].len;
    }
  }
  return pages;
}

static bool _create_controllable(anmem_config_t * config,
                                 anmem_t * mem,
                                 uint64_t pagesExp,
                                 uint64_t pageSkip) {
  uint64_t grabSize = 1 << pagesExp;
  uint64_t sizeRemaining = grabSize;
  
  while (grabSize >= 0x4 && sizeRemaining > 0) {
    uint64_t i, curPage = 0;
    void * structs = config->structs;
    for (i = 0; i < config->structCount && sizeRemaining > 0; i++) {
      // read the structure
      uint64_t fullSize = *((uint64_t *)(structs + config->sizeOffset));
      uint64_t physPage = *((uint64_t *)(structs + config->physPageOffset));
      structs += config->structSize;
      
      // calculate the available bounds
      uint64_t lowerBound = curPage, upperBound = curPage + fullSize;
#ifndef IGNORE_4GB_RULE
      if (upperBound + (physPage - lowerBound) > 0x100000) {
        upperBound = 0x100000 + (lowerBound - physPage);
      }
#endif
      if (lowerBound < pageSkip) {
        physPage += pageSkip - lowerBound;
        lowerBound = pageSkip;
      }
#ifndef IGNORE_4GB_RULE
      if (physPage >= 0x100000) break;
#endif
      
      curPage += fullSize;
      // while there are pages left to grab, grab available aligned regions
      while (sizeRemaining) {
        // make sure there is a possibility of finding a region
        if (lowerBound >= upperBound) break;
        if (upperBound - lowerBound < grabSize) break;
        
        // look for an aligned address between lowerBound and upperBound
        if (physPage % grabSize) {
          uint64_t nextBound = lowerBound + grabSize
            - (physPage % grabSize);
          if (nextBound + grabSize > upperBound) break;
          
          if (!_region_is_taken(mem, nextBound, grabSize)) {
            if (!_add_allocator(mem, nextBound, grabSize, 1)) {
              return false;
            }
            sizeRemaining -= grabSize;
          }
          
          physPage = nextBound + grabSize + (physPage - lowerBound);
          lowerBound = nextBound + grabSize;
        } else {
          // we found a region for our grab size
          if (!_region_is_taken(mem, lowerBound, grabSize)) {
            if (!_add_allocator(mem, lowerBound, grabSize, 1)) {
              return false;
            }
            sizeRemaining -= grabSize;
          }
          
          physPage += grabSize;
          lowerBound += grabSize;
        }
      }
    }
    grabSize >>= 1;
  }
  
  return true;
}

static bool _add_allocator(anmem_t * mem,
                           uint64_t start,
                           uint64_t len,
                           uint64_t type) {
  if (mem->count == mem->maximum) return false;
  mem->allocators[mem->count].type = type;
  mem->allocators[mem->count].start = start;
  mem->allocators[mem->count].len = len;
  mem->count++;
  return true;
}

static uint64_t _sum_regions(anmem_config_t * config) {
  uint64_t i, curPage = 0;
  void * structs = config->structs;
  for (i = 0; i < config->structCount; i++) {
    // read the structure
    uint64_t fullSize = *((uint64_t *)(structs + config->sizeOffset));
    structs += config->structSize;
    curPage += fullSize;
  }
  return curPage;
}

static uint64_t _next_analloc(anmem_t * mem, uint64_t start, uint64_t * len) {
  uint64_t i, firstPlace = 0xffffffffffffffffL;
  for (i = 0; i < mem->count; i++) {
    if (mem->allocators[i].start < firstPlace
        && mem->allocators[i].start >= start) {
      firstPlace = mem->allocators[i].start;
      *len = mem->allocators[i].len;
    }
  }
  return firstPlace;
}

static bool _region_is_taken(anmem_t * mem, uint64_t start, uint64_t len) {
  // see if any allocators overlap with the specified range
  uint64_t i;
  for (i = 0; i < mem->count; i++) {
    anmem_section_t seg = mem->allocators[i];
    if (seg.start >= start + len) continue;
    if (seg.start + seg.len <= start) continue;
    return true;
  }
  return false;
}
