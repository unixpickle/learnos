#include <stdio.h>
#include <assert.h>
#include <anmem/config.h>

void test_high_only();
void test_fragmented_enough();
void test_forced_align();
void test_alloc_overflow();
void test_physical_alignment();

int main() {
  test_high_only();
  test_fragmented_enough();
  test_forced_align();
  test_alloc_overflow();
  test_physical_alignment();
  return 0;
}

void test_high_only() {
  printf("testing >4GB only...");
  
  // there's more than enough memory, but it's all above 4GB (boohoo for you)
  uint64_t regions[] = {0x100, 0, 0xFFF00, 0x100, 0x100000, 0x100000};
  anmem_section_t sections[1];
  
  anmem_config_t config;
  config.structs = regions;
  config.sizeOffset = 0;
  config.physPageOffset = 8;
  config.structSize = 0x10;
  config.structCount = 3;
  
  anmem_t mem;
  mem.allocators = sections;
  mem.count = 0;
  mem.maximum = 1;
  
  // only requesting two pages of analloc, but it's above the limit
  bool result = anmem_configure(&config, &mem, 1, 0x100000);
  assert(result);
  
  assert(mem.count == 1);
  assert(mem.allocators[0].type == 0);
  assert(mem.allocators[0].start == 0x100000);
  assert(mem.allocators[0].len == 0x100000);
  
  printf(" passed!\n");
}

void test_fragmented_enough() {
  printf("testing fragmented but enough...");
  
  uint64_t regions[] = {
    0x100, 0,
    0x100, 0x100,
    0x200, 0x200,
    0x400, 0x400,
    0x800, 0x800
  };
  anmem_section_t sections[5];
  
  anmem_config_t config;
  config.structs = regions;
  config.sizeOffset = 0;
  config.physPageOffset = 8;
  config.structSize = 0x10;
  config.structCount = 5;
  
  anmem_t mem;
  mem.allocators = sections;
  mem.count = 0;
  mem.maximum = 5;
  
  // requesting 0x1000 pages = 16MB
  bool result = anmem_configure(&config, &mem, 12, 0);
  assert(result);
  
  assert(mem.count == 5);
  assert(mem.allocators[0].type == 1);
  assert(mem.allocators[1].type == 1);
  assert(mem.allocators[2].type == 1);
  assert(mem.allocators[3].type == 1);
  assert(mem.allocators[4].type == 1);
  assert(mem.allocators[0].start == 0x800);
  assert(mem.allocators[0].len == 0x800);
  assert(mem.allocators[1].start == 0x400);
  assert(mem.allocators[1].len == 0x400);
  assert(mem.allocators[2].start == 0x200);
  assert(mem.allocators[2].len == 0x200);
  assert(mem.allocators[3].start == 0);
  assert(mem.allocators[3].len == 0x100);
  assert(mem.allocators[4].start == 0x100);
  assert(mem.allocators[4].len == 0x100);
  
  printf(" passed!\n");
}

void test_forced_align() {
  printf("testing basic forced alignment...");
  
  // take an aligned 0x80 from the second region, then an aligned 0x40
  // then an aligned 0x40 from the third region
  uint64_t regions[] = {
    0x10, 0x0, 0x100, 0x10, 0x80, 0x110
  };
  
  anmem_section_t sections[6];
  
  anmem_config_t config;
  config.structs = regions;
  config.sizeOffset = 0;
  config.physPageOffset = 8;
  config.structSize = 0x10;
  config.structCount = 3;
  
  anmem_t mem;
  mem.allocators = sections;
  mem.count = 0;
  mem.maximum = 6;
  
  // requesting 0x100 pages
  bool result = anmem_configure(&config, &mem, 8, 0);
  assert(result);
  assert(mem.count == 6);
  
  assert(mem.allocators[0].type == 1);
  assert(mem.allocators[0].start == 0x80);
  assert(mem.allocators[0].len == 0x80);
  
  assert(mem.allocators[1].type == 1);
  assert(mem.allocators[1].start == 0x40);
  assert(mem.allocators[1].len == 0x40);
  
  assert(mem.allocators[2].type == 1);
  assert(mem.allocators[2].start == 0x140);
  assert(mem.allocators[2].len == 0x40);
  
  assert(mem.allocators[3].type == 0);
  assert(mem.allocators[3].start == 0);
  assert(mem.allocators[3].len == 0x40);
  
  assert(mem.allocators[4].type == 0);
  assert(mem.allocators[4].start == 0x100);
  assert(mem.allocators[4].len == 0x40);
  
  assert(mem.allocators[5].type == 0);
  assert(mem.allocators[5].start == 0x180);
  assert(mem.allocators[5].len == 0x10);
  
  printf(" passed!\n");
}

void test_alloc_overflow() {
  printf("testing allocator overflow...");
  
  uint64_t regions[] = {
    0x10, 0x0, 0x100, 0x10, 0x80, 0x110
  };
  
  anmem_section_t sections[5];
  
  anmem_config_t config;
  config.structs = regions;
  config.sizeOffset = 0;
  config.physPageOffset = 8;
  config.structSize = 0x10;
  config.structCount = 3;
  
  anmem_t mem;
  mem.allocators = sections;
  mem.count = 0;
  mem.maximum = 5;
  
  // requesting 0x100 pages
  bool result = anmem_configure(&config, &mem, 8, 0);
  assert(!result);
  assert(mem.count == 4);
  
  assert(mem.allocators[0].type == 1);
  assert(mem.allocators[0].start == 0x80);
  assert(mem.allocators[0].len == 0x80);
  
  assert(mem.allocators[1].type == 1);
  assert(mem.allocators[1].start == 0x40);
  assert(mem.allocators[1].len == 0x40);
  
  assert(mem.allocators[2].type == 0);
  assert(mem.allocators[2].start == 0);
  assert(mem.allocators[2].len == 0x40);
  
  assert(mem.allocators[3].type == 0);
  assert(mem.allocators[3].start == 0x100);
  assert(mem.allocators[3].len == 0x90);

  printf(" passed!\n");
}

void test_physical_alignment() {
  printf("testing *physical* alignment...");
  
  uint64_t regions[] = {
    0x10, 0,
    // 0x10 page physical gap
    0x20, 0x20,
    // 0x1 physical page gap
    0x81, 0x41,
    // 0x39 page physical gap
    0x101, 0x101
  };
  
  anmem_section_t sections[6];
  
  anmem_config_t config;
  config.structs = regions;
  config.sizeOffset = 0;
  config.physPageOffset = 8;
  config.structSize = 0x10;
  config.structCount = 4;
  
  anmem_t mem;
  mem.allocators = sections;
  mem.count = 0;
  mem.maximum = 6;
  
  bool result = anmem_configure(&config, &mem, 8, 0);
  assert(result);
  assert(mem.count == 6);
  
  // one 0x80 analloc at (0x180 phys, 0x10+0x20+0x81+0x7f vir)
  assert(mem.allocators[0].type == 1);
  assert(mem.allocators[0].len == 0x80);
  assert(mem.allocators[0].start == 0x130);
  // one 0x40 analloc at (0x80 phys, 0x10+0x20+0x3f vir)
  assert(mem.allocators[1].type == 1);
  assert(mem.allocators[1].len == 0x40);
  assert(mem.allocators[1].start == 0x6f);
  // one 0x40 analloc at (0x140 phys, 0x10+0x20+0x81+0x3f vir)
  assert(mem.allocators[2].type == 1);
  assert(mem.allocators[2].len == 0x40);
  assert(mem.allocators[2].start == 0xf0);
  // anpages of length 0x6f at (0 phys, 0 vir)
  assert(mem.allocators[3].type == 0);
  assert(mem.allocators[3].len == 0x6f);
  assert(mem.allocators[3].start == 0);
  // anpages of length 0xaf at (0xc0 phys, 0x10+0x20+0x3f+0x40 vir)
  assert(mem.allocators[4].type == 0);
  assert(mem.allocators[4].len == 0x41);
  assert(mem.allocators[4].start == 0xaf);
  // anpages of length 0x2 at (0x200 phys, 0x10+0x20+0x81+0x7f+0x80 vir)
  assert(mem.allocators[5].type == 0);
  assert(mem.allocators[5].len == 0x2);
  assert(mem.allocators[5].start == 0x1b0);
  
  printf(" passed!\n");
}
