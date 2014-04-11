#include <stdio.h>
#include <stdlib.h> // posix_memalign
#include <assert.h>
#include <anmem/config.h>
#include <anmem/alloc.h>

static anmem_t mem;
static anmem_section_t sections[3];
static void * buffer;

void test_initialize();
void test_alloc_pages();
void test_alloc_aligned();
void test_alloc_pages_overflow();

int main() {
  test_initialize();
  test_alloc_pages();
  test_alloc_aligned();
  test_alloc_pages_overflow();
  return 0;
}

void test_initialize() {
  printf("testing initialization...");
  
  // allocate 128K of aligned memory
  posix_memalign(&buffer, 0x20000, 0x20000);
  
  uint64_t firstPage = ((uint64_t)buffer) >> 12;
  uint64_t sizes[] = {firstPage, 0, 0x1, firstPage, 0x1f, firstPage + 1};
  
  anmem_config_t config;
  config.structs = sizes;
  config.sizeOffset = 0;
  config.physPageOffset = 8;
  config.structSize = 0x10;
  config.structCount = 3;
  
  mem.allocators = sections;
  mem.maximum = 3;
  mem.count = 0;
  
  // grab 8 pages of anlock
  bool result = anmem_configure(&config, &mem, 3, firstPage);
  assert(result);
  
  assert(mem.count == 3);
  assert(sections[0].type == 1);
  assert(sections[1].type == 0);
  assert(sections[2].type == 0);
  
  assert(sections[0].start == 8 + firstPage);
  assert(sections[0].len == 8);
  
  assert(sections[1].start == firstPage);
  assert(sections[1].len == 8);
  assert(sections[2].start == 0x10 + firstPage);
  assert(sections[2].len == 0x10);
  
  result = anmem_init_structures(&mem);
  assert(result);
  
  printf(" passed!\n");
}

void test_alloc_pages() {
  printf("testing anmem_alloc_page()...");
  
  uint64_t firstPage = ((uint64_t)buffer) >> 12;
  
  // allocating 0xf pages should all be from the top allocator
  uint64_t i;
  for (i = 0; i < 0xf; i++) {
    uint64_t page = ((uint64_t)anmem_alloc_page(&mem)) >> 12;
    assert(page == firstPage + 0x1f - i);
  }
  
  // allocating the next 7 pages should be from the first allocator
  for (i = 0; i < 7; i++) {
    uint64_t page = ((uint64_t)anmem_alloc_page(&mem)) >> 12;
    assert(page == firstPage + 7 - i);
  }
  
  printf(" passed!\n");
}

void test_alloc_aligned() {
  printf("testing anmem_alloc_aligned()...");
  
  uint64_t firstPage = ((uint64_t)buffer) >> 12;
  
  void * firstBuff = anmem_alloc_aligned(&mem, 4);
  assert(firstBuff != NULL);
  void * nextBuff = anmem_alloc_aligned(&mem, 2);
  assert(nextBuff != NULL);
  
  uint64_t page1 = ((uint64_t)firstBuff) >> 12;
  assert(page1 == firstPage + 0xc);
  uint64_t page2 = ((uint64_t)nextBuff) >> 12;
  assert(page2 == firstPage + 0xa);
  
  anmem_free_aligned(&mem, nextBuff, 4);
  anmem_free_aligned(&mem, firstBuff, 2);
  firstBuff = anmem_alloc_aligned(&mem, 4);
  page1 = ((uint64_t)firstBuff) >> 12;
  assert(page1 == firstPage + 0xc);
  
  anmem_free_aligned(&mem, firstBuff, 4);
  
  printf(" passed!\n");
}

void test_alloc_pages_overflow() {
  printf("testing allocation overflow...");
  
  uint64_t firstPage = ((uint64_t)buffer) >> 12;
  
  // right now, we have all of the single pages allocated, so the next page
  // we allocate should be from the analloc_t
  void * buffer = anmem_alloc_page(&mem);
  uint64_t bufferPage = ((uint64_t)buffer) >> 12;
  assert(bufferPage == firstPage + 0x9);
  
  buffer = anmem_alloc_page(&mem);
  bufferPage = ((uint64_t)buffer) >> 12;
  assert(bufferPage == firstPage + 0xa);
  
  anmem_free_page(&mem, (void *)((firstPage + 1) << 12));
  void * another = anmem_alloc_page(&mem);
  assert(another == (void *)((firstPage + 1) << 12));
  
  // there should be 5 pages left
  uint64_t i;
  for (i = 0; i < 5; i++) {
    buffer = anmem_alloc_page(&mem);
    bufferPage = ((uint64_t)buffer) >> 12;
    assert(bufferPage == firstPage + 0xb + i);
  }
  
  assert(anmem_alloc_page(&mem) == NULL);
  
  printf(" passed!\n");
}
