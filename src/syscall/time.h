#include <stdint.h>

/**
 * Return the current system time in microseconds.
 */
uint64_t syscall_get_time();

/**
 * Sleep for a specified number of microseconds.
 */
void syscall_sleep(uint64_t until);

/**
 * Find the thread with a given thread ID, and remove any sleep timeout it may
 * have set.
 */
void syscall_unsleep(uint64_t thread);

void syscall_clear_unsleep();

