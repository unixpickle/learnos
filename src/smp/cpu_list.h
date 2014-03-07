#include <stdint.h>
#include <anlock.h>
#include "task.h"
#include <smp/gdt.h>

typedef struct {
  uint32_t cpuId; // the x2APIC ID
  page_t baseStack; // page index of the CPUs 1-page stack
  page_t nextCPU; // 0 => this is the last one

  uint64_t lock;
  task_t * currentTask;
  thread_t * currentThread;

  tss_t * tss;
  uint16_t tssSelector;
} __attribute__((packed)) cpu_info;

void cpu_list_initialize(uint32_t firstProc);

cpu_info * cpu_list_lookup(uint32_t cpuId);
void cpu_list_add(uint64_t page);

cpu_info * cpu_get_current();

