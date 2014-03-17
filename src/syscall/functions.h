#include <stdint.h>

void syscall_print_method(void * ptr);
void syscall_sleep_method();
void syscall_getint_method();
void syscall_thread_exit_method();
void syscall_pid_kill_method(uint64_t pid);

