#include "stdlib.h"
#include <base/system.h>

void abort() {
  sys_abort();
}

int abs(int num) {
  return num < 0 ? -num : num;
}

