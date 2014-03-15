#include "cpu.h"
#include <interrupts/lapic.h>

static cpu_t * firstCPU = NULL;

void cpu_add(cpu_t * cpu) {
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
  cpu_t * this = cpu_get_current();
  if (!this) return NULL;
  return (this->baseStack + 1) << 12;
}

