#include "cpu_config.h"
#include <interrupts/lapic.h>
#include <libkern_base.h>
#include <kernpage.h>

void configure_cpu(uint64_t stack) {
  kernpage_lock();
  uint64_t entryPage = kernpage_alloc_virtual();
  kernpage_unlock();
  cpu_info * info = (cpu_info *)(entryPage << 12);
  info->cpuId = lapic_get_id();
  info->baseStack = stack;
  info->threadCur = 0;
  info->nextCPU = 0;
  cpu_list_add(entryPage);
}

void task_loop() {
  halt();
}

void smp_entry(uint64_t stack) {
  lapic_enable();
  lapic_set_defaults();
  lapic_set_priority(0x0);

  // configure the CPU
  configure_cpu(stack);
  task_loop();
}

