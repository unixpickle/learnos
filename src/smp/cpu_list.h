#include <stdint.h>

typedef struct {
  uint32_t cpuId; // the x2APIC ID
  uint64_t baseStack; // page index of the CPUs 1-page stack
  uint64_t threadCur; // 0 = idle
  uint64_t nextCPU; // 0 => this is the last one
} __attribute__((packed)) cpu_info;

void cpu_list_initialize(uint32_t firstProc);

cpu_info * cpu_list_lookup(uint32_t cpuId);
void cpu_list_add(uint64_t page);

