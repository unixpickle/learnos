#include "cpu_config.h"
#include <interrupts/lapic.h>
#include <libkern_base.h>
#include <kernpage.h>
#include "scheduler.h"
#include <interrupts/idt.h>
#include <interrupts/basic.h>

void configure_cpu(uint64_t stack) {
  kernpage_lock();
  uint64_t entryPage = kernpage_alloc_virtual();
  kernpage_unlock();
  cpu_info * info = (cpu_info *)(entryPage << 12);
  info->cpuId = lapic_get_id();
  info->baseStack = stack;
  info->currentThread = NULL;
  info->currentTask = NULL;
  info->nextCPU = 0;
  info->tssSelector = (uint16_t)gdt_get_size();
  info->tss = gdt_add_tss();
  cpu_list_add(entryPage);
}

void task_loop() {
  asm("int %0" : : "i" (IDT_VECTOR_TIMER));
  hang();
}

void smp_entry(uint64_t stack) {
  lapic_enable();
  lapic_set_defaults();
  lapic_set_priority(0x0);

  load_idtr((void *)IDTR_PTR);

  // configure the CPU
  configure_cpu(stack);
  task_loop();
}

