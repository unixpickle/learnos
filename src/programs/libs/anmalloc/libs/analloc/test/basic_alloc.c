#include "test.h"
#include <stdlib.h>
#include <analloc.h>

int test_initialize();
int test_alloc_low();
int test_alloc_high();
int test_overflow();

static uint8_t * buffer;
static uint32_t * map;
static uint32_t initialMap = 0b10000000001000011011;

int main(int argc, const char * argv[]) {
  test_t tests[] = {
    {test_initialize, "analloc_with_chunk", 1},
    {test_alloc_low, "analloc_alloc (low)", 1},
    {test_alloc_high, "analloc_alloc (high)", 1},
    {test_overflow, "analloc_alloc (overflow)", 1}
  };
  test_ensure_64_bit();
  return test_run_all(tests, 4);
}

int test_initialize() {
  // each page is 8 bytes, total depth of 4
  uint64_t infoSize = 0x20;
  buffer = (uint8_t *)malloc(0x80);
  map = (uint32_t *)&buffer[0x20];

  uint8_t res = analloc_with_chunk((analloc_t)buffer,
                                   buffer,
                                   0x80,
                                   infoSize,
                                   8);
  if (!res) return 1;
  if (*map != initialMap) return 2;
  return 0;
}

int test_alloc_low() {
  uint64_t size = 8;
  void * ptr = analloc_alloc((analloc_t)buffer, &size, 0);
  if (size != 8) return 1;
  if (ptr != (void *)&buffer[0x28]) return 2;
  if (*map != 0b110000000001000011011) return 3;
  analloc_free((analloc_t)buffer, ptr, 8);
  if (*map != initialMap) return 4;
  return 0;
}

int test_alloc_high() {
  uint64_t size = 0x10;
  void * ptr = analloc_alloc((analloc_t)buffer, &size, 1);
  if (size != 0x10) return 1;
  if (ptr != (void *)&buffer[0x70]) return 2;
  if (*map != 0b10000100001001011111) return 3;
  analloc_free((analloc_t)buffer, ptr, 8);
  if (*map != initialMap) return 4;
  return 0;
}

int test_overflow() {
  uint64_t size = 0x40;

  // allocate a big buffer (should succeed)
  void * ptr = analloc_alloc((analloc_t)buffer, &size, 1);
  if (size != 0x40) return 1;
  if (ptr != (void *)&buffer[0x40]) return 2;
  if (*map != 0b10000000001000011111) return 3;

  // try allocating another big one (should fail)
  void * ptr2 = analloc_alloc((analloc_t)buffer, &size, 1);
  if (ptr2 != 0 || size != 0) return 4;

  // we have 0x1c bytes free, an 8 and a 0x10
  size = 0x10;
  void * ptr3 = analloc_alloc((analloc_t)buffer, &size, 1);
  if (size != 0x10) return 5;
  if (ptr3 != (void *)&buffer[0x30]) return 6;
  if (*map != 0b10000000011000011111) return 7;

  // try allocating another 0x10 one (should fail)
  ptr2 = analloc_alloc((analloc_t)buffer, &size, 1);
  if (ptr2 != 0 || size != 0) return 8;

  // we have 0x8 bytes left and we'll utilize them here
  size = 8;
  void * ptr4 = analloc_alloc((analloc_t)buffer, &size, 1);
  if (ptr4 != (void *)&buffer[0x28]) return 9;
  if (*map != 0b110000000011000011111) return 10;

  analloc_free((analloc_t)buffer, ptr, 0x40);
  analloc_free((analloc_t)buffer, ptr3, 0x10);
  analloc_free((analloc_t)buffer, ptr4, 0x8);
  if (*map != initialMap) return 11;

  return 0;
}
