#include "cpu_list.h"
#include <anlock.h>
#include <stdio.h>
#include <kernpage.h>
#include <shared/addresses.h>

static void cpu_list_lock();
static void cpu_list_unlock();

void cpu_list_initialize(uint32_t cpuId) {
  kernpage_lock();
  uint64_t page = kernpage_alloc_virtual();
  uint64_t baseStack = kernpage_alloc_virtual();
  kernpage_unlock();
  if (!page || !baseStack) die("failed to allocate root CPU");
  cpu_info * info = (cpu_info *)(page << 12);
  info->cpuId = cpuId;
  info->baseStack = baseStack;
  info->threadCur = 0;
  info->nextCPU = 0;
  CPU_INFO_FIRST = page;
  anlock_initialize((anlock_t)CPU_LIST_LOCK);
}

cpu_info * cpu_list_lookup(uint32_t cpuId) {
  cpu_list_lock();
  cpu_info * cur = (cpu_info *)(CPU_INFO_FIRST << 12);
  while (cur->nextCPU) {
    if (cur->cpuId == cpuId) {
      cpu_list_unlock();
      return cur;
    }
    cur = (cpu_info *)(cur->nextCPU << 12);
  }
  if (cur->cpuId == cpuId) {
    cpu_list_unlock();
    return cur;
  }
  cpu_list_unlock();
  return NULL;
}

void cpu_list_add(uint64_t page) {
  cpu_list_lock();
  cpu_info * cur = (cpu_info *)(CPU_INFO_FIRST << 12);
  while (cur->nextCPU) {
    cur = (cpu_info *)(cur->nextCPU << 12);
  }
  cur->nextCPU = page;
  cpu_list_unlock();
}

static void cpu_list_lock() {
  anlock_t lock = (anlock_t)CPU_LIST_LOCK;
  anlock_lock(lock);
}

static void cpu_list_unlock() {
  anlock_t lock = (anlock_t)CPU_LIST_LOCK;
  anlock_unlock(lock);
}

