#include "test.h"
#include <stdlib.h>
#include <analloc.h>

static uint8_t * buffer;
static analloc_t alloc;

int test_mem_size();
int test_mem_size_offset();
int test_mem_start();
int test_mem_start_offset();

int main(int argc, const char * argv[]) {
  test_t tests[] = {
    {test_mem_size, "analloc_mem_size() - basic", 1},
    {test_mem_size_offset, "analloc_mem_size() - offset", 1},
    {test_mem_start, "analloc_mem_start() - basic", 1},
    {test_mem_start_offset, "analloc_mem_start() - offset", 1}
  };
  test_ensure_64_bit();
  
  buffer = (uint8_t *)malloc(0x100000);
  alloc = (analloc_t)buffer;
  analloc_with_chunk(alloc, buffer, 0x100000,
                     sizeof(analloc_struct_t), 0x10);
  
  int res = test_run_all(tests, 4 );
  free(buffer);
  return res;
}

int test_mem_size() {
  uint64_t inOut = 0x100;
  void * buff = analloc_alloc(alloc, &inOut, 0);
  if (analloc_mem_size(alloc, buff) != 0x100) return 1;
  analloc_free(alloc, buff, inOut);
  
  inOut = 0x200;
  buff = analloc_alloc(alloc, &inOut, 0);
  if (analloc_mem_size(alloc, buff) != 0x200) return 2;
  analloc_free(alloc, buff, inOut);
  
  return 0;
}

int test_mem_size_offset() {
  uint64_t inOut = 0x100;
  void * buff = analloc_alloc(alloc, &inOut, 0);
  if (analloc_mem_size(alloc, buff) != 0x100) return 1;
  if (analloc_mem_size(alloc, buff + 7) != 0x100) return 2;
  if (analloc_mem_size(alloc, buff + 0x40) != 0x100) return 3;
  if (analloc_mem_size(alloc, buff + 0xff) != 0x100) return 4;
  analloc_free(alloc, buff, inOut);
  
  inOut = 0x200;
  buff = analloc_alloc(alloc, &inOut, 0);
  if (analloc_mem_size(alloc, buff) != 0x200) return 5;
  if (analloc_mem_size(alloc, buff + 7) != 0x200) return 6;
  if (analloc_mem_size(alloc, buff + 0x73) != 0x200) return 7;
  if (analloc_mem_size(alloc, buff + 0x1ff) != 0x200) return 8;
  analloc_free(alloc, buff, inOut);
  
  return 0;
}

int test_mem_start() {
  uint64_t inOut = 1;
  void * buff = analloc_alloc(alloc, &inOut, 1);
  if (analloc_mem_start(alloc, buff) != buff) return 1;
  analloc_free(alloc, buff, inOut);
  
  inOut = 0x80000;
  buff = analloc_alloc(alloc, &inOut, 0);
  if (analloc_mem_start(alloc, buff) != buff) return 2;
  analloc_free(alloc, buff, inOut);
  
  return 0;
}

int test_mem_start_offset() {
  uint64_t inOut = 1;
  void * buff = analloc_alloc(alloc, &inOut, 1);
  if (analloc_mem_start(alloc, buff) != buff) return 1;
  if (analloc_mem_start(alloc, buff + 7) != buff) return 3;
  if (analloc_mem_start(alloc, buff + 0xf) != buff) return 3;
  if (analloc_mem_start(alloc, buff + 0x10) == buff) return 4;
  analloc_free(alloc, buff, inOut);
  
  inOut = 0x80000;
  buff = analloc_alloc(alloc, &inOut, 0);
  if (analloc_mem_start(alloc, buff) != buff) return 5;
  if (analloc_mem_start(alloc, buff + 0x10) != buff) return 6;
  if (analloc_mem_start(alloc, buff + 0x11) != buff) return 7;
  if (analloc_mem_start(alloc, buff + 0x10000) != buff) return 8;
  if (analloc_mem_start(alloc, buff + 0x50000) != buff) return 9;
  if (analloc_mem_start(alloc, buff + 0x7ffff) != buff) return 10;
  if (analloc_mem_start(alloc, buff - 1) == buff) return 11;
  analloc_free(alloc, buff, inOut);
  
  return 0;
}