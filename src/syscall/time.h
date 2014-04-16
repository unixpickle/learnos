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

/**
 * Atomically checks the value at the virtual address pointed to by condPtr. If
 * the address contains a 1, it is set to 0 and the thread is slept. Otherwise,
 * the value remains 0 and the thread is not slept.
 * @return 1 when the thread was slept, 0 when it was already woken up
 * @discussion The pointer you pass must be 8-byte aligned or else you will die.
 */
uint64_t syscall_atomic_sleep(uint64_t usec, uint64_t condPtr);

