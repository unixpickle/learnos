#include <stdint.h>

/**
 * Launches a new task, opens a socket to it, and returns the file descriptor in
 * the current task's descriptor space.
 * @noncritical
 */
uint64_t syscall_fork(uint64_t rip);

/**
 * Kill a task with a specified PID. This will only take effect if the given
 * task is running under the same UID or the current UID is 0.
 */
bool syscall_kill(uint64_t pid);

