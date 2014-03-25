#include <stdint.h>

uint64_t syscall_entry(uint64_t arg1, uint64_t arg2, uint64_t arg3);

void syscall_print(void * ptr);
uint64_t syscall_get_time();
void syscall_sleep(uint64_t until);

