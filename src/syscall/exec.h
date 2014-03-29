#include <stdint.h>

/**
 * Launches a new task, opens a socket to it, and returns the file descriptor in
 * the current task's descriptor space.
 * @noncritical
 */
uint64_t syscall_fork(uint64_t rip);

