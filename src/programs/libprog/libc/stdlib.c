#include "stdlib.h"
#include <base/system.h>

void abort() {
  sys_exit();
}

int abs(int num) {
  return num < 0 ? -num : num;
}

