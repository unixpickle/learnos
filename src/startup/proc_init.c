#include "proc_init.h"
#include <scheduler/cpu.h>
#include <anscheduler/functions.h>
#include <anscheduler/loop.h>
#include <interrupts/lapic.h>
#include <interrupts/basic.h>
#include <shared/addresses.h>
#include <syscall/config.h>

static void load_tss();
extern void load_new_gdt();

void proc_initialize(page_t stack) {
  proc_configure_basics();
  cpu_add_current(stack);
  proc_run_scheduler();
}

void proc_configure_basics() {
  lapic_enable();
  lapic_set_defaults();
  lapic_set_priority(0);
  load_idtr((void *)IDTR_PTR);
}

void proc_run_scheduler() {
  load_new_gdt();
  load_tss();
  syscall_initialize();
  anscheduler_loop_run();
}

static void load_tss() {
  cpu_t * cpu = cpu_current();
  uint16_t currentTss;
  __asm__ ("str %0" : "=r" (currentTss));
  if (currentTss != cpu->tssSelector) {
    __asm__ ("ltr %0" : : "r" (cpu->tssSelector));
  }
}

