#include "cpu.h"
#include <interrupts/lapic.h>
#include <kernpage.h>
#include <libkern_base.h>

static cpu_t * firstCPU = NULL;
static uint64_t count = 0;

uint64_t cpu_count() {
  return count;
}

void cpu_add(cpu_t * cpu) {
  cpu->task = NULL;
  cpu->thread = NULL;
  count++;
  if (!firstCPU) {
    firstCPU = cpu;
  } else {
    cpu_t * aCPU = firstCPU;
    while (aCPU->next) {
      aCPU = aCPU->next;
    }
    aCPU->next = cpu;
  }
}

void cpu_add_current(page_t stack) {
  kernpage_lock();
  uint64_t page = kernpage_alloc_virtual();
  kernpage_unlock();

  if (!page) return;
  zero_page(page);

  cpu_t * cpu = (cpu_t *)(page << 12);
  cpu->cpuId = lapic_get_id();
  cpu->baseStack = stack;
  cpu->tssSelector = (uint16_t)gdt_get_size();
  cpu->tss = gdt_add_tss();
  cpu_add(cpu);
}

cpu_t * cpu_lookup(uint32_t ident) {
  cpu_t * cpu = firstCPU;
  while (cpu) {
    if (cpu->cpuId == ident) return cpu;
    cpu = cpu->next;
  }
  return NULL;
}

cpu_t * cpu_current() {
  return cpu_lookup(lapic_get_id());
}

void * cpu_dedicated_stack() {
  cpu_t * cpu = cpu_current();
  if (!cpu) return NULL;
  return (void *)((cpu->baseStack + 1) << 12);
}

/***********************
 * anscheduler methods *
 ***********************/

void anscheduler_cpu_lock() {
  __asm__("cli");
}

void anscheduler_cpu_unlock() {
  __asm__("sti");
}

task_t * anscheduler_cpu_get_task() {
  cpu_t * cpu = cpu_current();
  return cpu->task;
}

thread_t * anscheduler_cpu_get_thread() {
  cpu_t * cpu = cpu_current();
  return cpu->thread;
}

void anscheduler_cpu_set_task(task_t * task) {
  cpu_t * cpu = cpu_current();
  cpu->task = task;
}

void anscheduler_cpu_set_thread(thread_t * thread) {
  cpu_t * cpu = cpu_current();
  cpu->thread = thread;
}

void anscheduler_cpu_notify_invlpg(task_t * task) {
  cpu_t * cpu = firstCPU;
  while (cpu) {
    if (cpu->task == task) {
      lapic_send_ipi(cpu->cpuId, 0x20, 0, 1, 0); // real simple IPI
    }
    cpu = cpu->next;
  }
}

void anscheduler_cpu_notify_dead(task_t * task) {
  cpu_t * cpu = firstCPU;
  while (cpu) {
    if (cpu->task == task) {
      lapic_send_ipi(cpu->cpuId, 0x20, 0, 1, 0); // real simple IPI
    }
    cpu = cpu->next;
  }
}

void anscheduler_cpu_stack_run(void * arg, void (* fn)(void *)) {
  void * stack = cpu_dedicated_stack();
  __asm__("mov %%rax, %%rsp\n"
          "callq *%%rbx" : : "a" (stack), "b" (fn), "D" (arg));
}

void anscheduler_cpu_halt() {
  // if only every function were THIS easy!
  __asm__("hlt");
}

