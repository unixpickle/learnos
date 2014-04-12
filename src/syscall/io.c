#include <anscheduler/functions.h>
#include <anscheduler/task.h>
#include "functions.h"

uint64_t syscall_in(uint64_t pin, uint64_t size) {
  uint64_t result = 0;

  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  if (task->uid) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }

  switch (size) {
    case 1:
      __asm__("xor %%rax, %%rax\n"
              "inb %%dx, %%al" : "=a" (result) : "d" (pin));
      break;
    case 2:
      __asm__("xor %%rax, %%rax\n"
              "inw %%dx, %%ax" : "=a" (result) : "d" (pin));
      break;
    case 4:
      __asm__("xor %%rax, %%rax\n"
              "inl %%dx, %%eax" : "=a" (result) : "d" (pin));
      break;
    default:
      break;
  }
  anscheduler_cpu_unlock();
  return result;
}

void syscall_out(uint64_t pin, uint64_t value, uint64_t size) {
  anscheduler_cpu_lock();
  task_t * task = anscheduler_cpu_get_task();
  if (task->uid) {
    anscheduler_task_exit(ANSCHEDULER_TASK_KILL_REASON_ACCESS);
  }
  switch (size) {
    case 1:
      __asm__("outb %%al, %%dx" : : "a" (value), "d" (pin));
      break;
    case 2:
      __asm__("outw %%ax, %%dx" : : "a" (value), "d" (pin));
      break;
    case 4:
      __asm__("outl %%eax, %%dx" : : "a" (value), "d" (pin));
      break;
    default:
      break;
  }
  anscheduler_cpu_unlock();
}

