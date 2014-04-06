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

/**
 * Call a function, passing in two arguments.
 */
void syscall_thread_launch(uint64_t rip, uint64_t arg1, uint64_t arg2);

/**
 * Exit the current thread.
 */
void syscall_thread_exit();

/**
 * Get the identifier of the current thread. This is an integer starting at 0,
 * and it will always be different for different threads in a task. It will
 * increment, and values will be re-used as threads die.
 */
uint64_t syscall_thread_id();

