#include "test.h"

void test_ensure_64_bit() {
  if (sizeof(void *) != 8) {
    fprintf(stderr, "Tests require 64 bit!\n");
    exit(1);
  }
}

int test_run_all(const test_t * tests, int count) {
  int i, ret = 0;
  for (i = 0; i < count; i++) {
    printf("Testing %s... ", tests[i].name);
    int result = tests[i].function();
    if (!result) {
      printf("passed!\n");
    } else {
      ret = result;
      printf("failed (%d)\n", result);
      if (tests[i].endAll) return result;
    }
  }
  return ret;
}
