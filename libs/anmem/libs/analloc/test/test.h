#include <stdio.h>
#include <stdlib.h>

typedef struct {
  int (* function)();
  const char * name;
  int endAll;
} test_t;

void test_ensure_64_bit();
int test_run_all(const test_t * tests, int count);
