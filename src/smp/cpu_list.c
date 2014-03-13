#include "cpu_list.h"
#include <anlock.h>
#include <stdio.h>
#include <kernpage.h>
#include <interrupts/lapic.h>
#include <shared/addresses.h>
#include "gdt.h"

static uint64_t cpuListLock;
static cpu_info * cpuInfoFirst;

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
  info->nextCPU = 0;
  info->lock = 0;
  info->currentThread = NULL;
  info->currentTask = NULL;
  cpuInfoFirst = info;
  anlock_initialize(&cpuListLock);

  info->tssSelector = (uint16_t)gdt_get_size();
  info->tss = gdt_add_tss();
}

cpu_info * cpu_list_lookup(uint32_t cpuId) {
  cpu_list_lock();
  cpu_info * cur = cpuInfoFirst;
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
  cpu_info * cur = cpuInfoFirst;
  while (cur->nextCPU) {
    cur = (cpu_info *)(cur->nextCPU << 12);
  }
  cur->nextCPU = page;
  cpu_list_unlock();
}

cpu_info * cpu_get_current() {
  uint32_t ident = lapic_get_id();
  return cpu_list_lookup(ident);
}

void * cpu_get_dedicated_stack() {
  cpu_info * info = cpu_get_current();
  return (void *)((info->baseStack + 1) << 0xc);
}

static void cpu_list_lock() {
  anlock_lock(&cpuListLock);
}

static void cpu_list_unlock() {
  anlock_unlock(&cpuListLock);
}

