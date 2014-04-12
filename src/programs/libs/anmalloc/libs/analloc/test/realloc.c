#include "test.h"
#include <stdlib.h>
#include <analloc.h>

int test_initialize();
int test_realloc_low();
int test_realloc_high();

static uint8_t * buffer;
static uint32_t * map;
static uint32_t initialMap = 0b1000000010001011;

int main(int argc, const char * argv[]) {
  test_t tests[] = {
    {test_initialize, "analloc_with_chunk", 1},
    {test_realloc_low, "analloc_realloc() - low", 1},
    {test_realloc_high, "analloc_realloc() - high", 1}
  };
  test_ensure_64_bit();
  return test_run_all(tests, 3);
}

int test_initialize() {
  // each page is 0x24 bytes, total depth of 4
  uint64_t infoSize = 0x20;
  buffer = (uint8_t *)malloc(0x240);
  map = (uint32_t *)&buffer[0x20];

  uint8_t res = analloc_with_chunk((analloc_t)buffer,
                                   buffer,
                                   0x240,
                                   infoSize,
                                   0x24);
  if (!res) return 1;
  if (*map != initialMap) return 2;
  return 0;
}

int test_realloc_low() {
  uint64_t size = 0x24;
  uint8_t * ptr = (uint8_t *)analloc_alloc((analloc_t)buffer, &size, 0);
  if (!ptr || size != 0x24) return 1;
  if (ptr != buffer + 0x24) return 2;
  if (*map != 0b11000000010001011) return 3;
  if (analloc_mem_size((analloc_t)buffer, ptr) != size) return 4;
  
  size = 0x48;
  uint8_t * ptr2 = (uint8_t *)analloc_alloc((analloc_t)buffer, &size, 0);
  if (!ptr2 || size != 0x48) return 5;
  if (ptr2 != buffer + 0x48) return 6;
  if (*map != 0b11000000110001011) return 7;
  if (analloc_mem_size((analloc_t)buffer, ptr2) != size) return 8;
  
  size = 0x90;
  uint8_t * ptr2New = (uint8_t *)analloc_realloc((analloc_t)buffer,
                                                 ptr2, 0x48, &size, 0);
  if (!ptr2New || size != 0x90) return 9;
  if (ptr2New != buffer + 0x90) return 10;
  if (*map != 0b11000000010011011) return 11;
  if (analloc_mem_size((analloc_t)buffer, ptr2New) != size) return 12;
  
  analloc_free((analloc_t)buffer, ptr2New, 0x90);
  if (*map != 0b11000000010001011) return 13;
  
  size = 0x48;
  uint8_t * ptrNew = (uint8_t *)analloc_realloc((analloc_t)buffer,
                                                ptr, 0x24, &size, 0);
  if (!ptrNew || size != 0x48) return 14;
  if (ptrNew != buffer + 0x48) return 15;
  if (*map != 0b1000000110001011) return 16;
  if (analloc_mem_size((analloc_t)buffer, ptrNew) != size) return 17;
  
  analloc_free((analloc_t)buffer, ptrNew, 0x48);
  if (*map != initialMap) return 18;
  
  return 0;
}

int test_realloc_high() {
  // here I use arbitrary sizes for allocation requests to make sure
  // that rounding up to the nearest power of 2 works.
  
  uint64_t size = 0x23; // really 0x24
  uint8_t * ptr = (uint8_t *)analloc_alloc((analloc_t)buffer, &size, 1);
  if (ptr != buffer + 0x240 - 0x24) return 1;
  
  size = 0x42; // really 0x48
  uint8_t * ptr2 = (uint8_t *)analloc_realloc((analloc_t)buffer, ptr,
                                              0x24, &size, 1);
  if (ptr2 != buffer + 0x240 - 0x48) return 2;
  
  // try allocating too much and fail
  size = 0x240;
  uint32_t beforeFail = *map;
  void * fail = analloc_realloc((analloc_t)buffer, ptr2, 0x120, &size, 1);
  if (fail || size != 0x0 || *map != beforeFail) return 3;
  
  size = 0x88; // really 0x90
  uint8_t * ptr3 = (uint8_t *)analloc_realloc((analloc_t)buffer, ptr2,
                                              0x48, &size, 1);
  if (ptr3 != buffer + 0x240 - 0x90) return 4;
  
  size = 0x110; // really 0x120
  uint8_t * ptr4 = (uint8_t *)analloc_realloc((analloc_t)buffer, ptr3,
                                              0x90, &size, 1);
  if (ptr4 != buffer + 0x120) return 5;
  
  size = 0x240;
  beforeFail = *map;
  fail = (uint8_t *)analloc_realloc((analloc_t)buffer, ptr4, 0x120, &size, 1);
  if (fail || size != 0x0 || *map != beforeFail) return 6;
  
  analloc_free((analloc_t)buffer, ptr4, 0x120);
  if (*map != initialMap) return 7;
  
  return 0;
}
