#include <stdint.h>

typedef struct {
  uint64_t rax;
  uint64_t rbp;
  uint64_t rbx;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;
  uint64_t rip;
  uint64_t rsp;
  uint64_t cr3;
} __attribute__((packed)) restore_regs;

uint64_t syscall_entry(uint64_t arg1, uint64_t arg2, uint64_t arg3);
void syscall_return(restore_regs * regs);

void syscall_print(void * ptr);
uint64_t syscall_get_time();
void syscall_sleep(uint64_t until);

