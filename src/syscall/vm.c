#include "vm.h"
#include <anscheduler/functions.h>
#include <anscheduler/task.h>

static bool _validate_stack_addr(const void * tPtr);

bool task_copy_in(void * kPointer, const void * tPointer, uint64_t len) {
  if (!_validate_stack_addr(tPointer)) return false;

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
    if (end == 0) break;

    anscheduler_lock(&task->vmLock);
    uint16_t flags;
    uint64_t entry = anscheduler_vm_lookup(task->vm, i, &flags);
    if (flags & ANSCHEDULER_PAGE_FLAG_UNALLOC) {
      flags = ANSCHEDULER_PAGE_FLAG_USER
        | ANSCHEDULER_PAGE_FLAG_PRESENT
        | ANSCHEDULER_PAGE_FLAG_WRITE;
      void * ptr = anscheduler_alloc(0x1000);
      anscheduler_zero(ptr, 0x1000);
      entry = anscheduler_vm_physical(((uint64_t)ptr) >> 12);
      anscheduler_vm_map(task->vm, i, entry, flags);
    }
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
  if (!_validate_stack_addr(tPointer)) return false;
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
    if (flags & ANSCHEDULER_PAGE_FLAG_UNALLOC) {
      flags = ANSCHEDULER_PAGE_FLAG_USER
        | ANSCHEDULER_PAGE_FLAG_PRESENT
        | ANSCHEDULER_PAGE_FLAG_WRITE;
      void * ptr = anscheduler_alloc(0x1000);
      anscheduler_zero(ptr, 0x1000);
      entry = anscheduler_vm_physical(((uint64_t)ptr) >> 12);
      anscheduler_vm_map(task->vm, i, entry, flags);
    }
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
      dest[j + start] = source[j];
    }
    kPointer += end - start;
  }
  return true;
}

bool task_get_virtual(const void * tPtr, void ** out) {
  if (!_validate_stack_addr(tPtr)) return false;

  task_t * task = anscheduler_cpu_get_task();
  uint64_t pageIdx = ((uint64_t)tPtr) >> 12;

  anscheduler_lock(&task->vmLock);
  uint16_t flags;
  uint64_t entry = anscheduler_vm_lookup(task->vm, pageIdx, &flags);
  if (flags & ANSCHEDULER_PAGE_FLAG_UNALLOC) {
    flags = ANSCHEDULER_PAGE_FLAG_USER
      | ANSCHEDULER_PAGE_FLAG_PRESENT
      | ANSCHEDULER_PAGE_FLAG_WRITE;
    void * ptr = anscheduler_alloc(0x1000);
    anscheduler_zero(ptr, 0x1000);
    entry = anscheduler_vm_physical(((uint64_t)ptr) >> 12);
    anscheduler_vm_map(task->vm, pageIdx, entry, flags);
  }
  anscheduler_unlock(&task->vmLock);
  if (!(flags & ANSCHEDULER_PAGE_FLAG_PRESENT)
      || !(flags & ANSCHEDULER_PAGE_FLAG_USER)
      || !(flags & ANSCHEDULER_PAGE_FLAG_WRITE)) {
    return false;
  }

  uint64_t vPage = anscheduler_vm_virtual(entry);
  *out = (void *)((vPage << 12) + (((uint64_t)tPtr) & 0xfff));
  return true;
}

static bool _validate_stack_addr(const void * tPtr) {
  thread_t * th = anscheduler_cpu_get_thread();
  uint64_t idx = th->stack;
  uint64_t base = (idx << 8) + ANSCHEDULER_TASK_USER_STACKS_PAGE;
  uint64_t thePage = ((uint64_t)tPtr) >> 12;
  return thePage >= base && thePage < base + 0x100;
}

