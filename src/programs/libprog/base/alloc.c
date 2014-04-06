#include "alloc.h"
#include "system.h"

//static uint64_t pageSocket = UINT64_MAX;

bool alloc_pages(uint64_t pageIndex, uint64_t count) {
  if (sys_self_pid() == 0) {
    // do manual allocation
  }
  return false;
}

bool free_pages(uint64_t pageIndex, uint64_t count) {
  if (sys_self_pid() == 0) {
    // do manual free
  }
  return false;
}

