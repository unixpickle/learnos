#include "vm.h"
#include <anscheduler/functions.h>

bool task_copy_in(void * kPointer, const void * tPointer, uint64_t len) {
  task_t * task = anscheduler_cpu_get_task();

  uint64_t firstPage = ((uint64_t)tPointer) >> 12;
  uint64_t startOffset = ((uint64_t)tPointer) & 0xfff;
  uint64_t finalPage = ((uint64_t)tPointer + len) >> 12;
  uint64_t finalOffset = ((uint64_t)tPointer + len) & 0xfff;
  uint64_t i;
  for (i = firstPage; i <= finalPage; i++) {
    uint64_t start = 0, end = 0x1000;
    if (i == firstPage) start = startOffset;
    if (i == finalPage) end = finalOffset;
    anscheduler_lock(&task->vmLock);
    uint16_t flags;
    uint64_t entry = anscheduler_vm_lookup(task->vm, i, &flags);
    anscheduler_unlock(&task->vmLock);
    if (!(flags & ANSCHEDULER_PAGE_FLAG_PRESENT)
        || !(flags & ANSCHEDULER_PAGE_FLAG_USER)) {
      return false;
    }
    uint8_t * source = (uint8_t *)(anscheduler_vm_virtual(entry) << 12);
    uint8_t * dest = kPointer;
    uint64_t j;
    for (j = 0; j < end - start; j++) {
      dest[j] = source[start + j];
    }
    kPointer += end - start;
  }
  return true;
}

bool task_copy_out(void * tPointer, const void * kPointer, uint64_t len) {
  task_t * task = anscheduler_cpu_get_task();

  uint64_t firstPage = ((uint64_t)tPointer) >> 12;
  uint64_t startOffset = ((uint64_t)tPointer) & 0xfff;
  uint64_t finalPage = ((uint64_t)tPointer + len) >> 12;
  uint64_t finalOffset = ((uint64_t)tPointer + len) & 0xfff;

  uint64_t i;
  for (i = firstPage; i <= finalPage; i++) {
    uint64_t start = 0, end = 0x1000;
    if (i == firstPage) start = startOffset;
    if (i == finalPage) end = finalOffset;
    anscheduler_lock(&task->vmLock);
    uint16_t flags;
    uint64_t entry = anscheduler_vm_lookup(task->vm, i, &flags);
    anscheduler_unlock(&task->vmLock);
    if (!(flags & ANSCHEDULER_PAGE_FLAG_PRESENT)
        || !(flags & ANSCHEDULER_PAGE_FLAG_USER)
        || !(flags & ANSCHEDULER_PAGE_FLAG_WRITE)) {
      return false;
    }
    uint8_t * dest = (uint8_t *)(anscheduler_vm_virtual(entry) << 12);
    const uint8_t * source = kPointer;
    uint64_t j;
    for (j = 0; j < end - start; j++) {
      dest[j] = source[start + j];
    }
    kPointer += end - start;
  }
  return true;
}

