#include "types.h"

/**
 * Frees a thread's kernel stack and the thread itself, doing little else.
 */
void thread_dealloc(thread_t * thread);

/**
 * Frees a task's resources. Since this should be called from a critical
 * section, it is recommended that the caller cleans up the task's resources
 * first, from outside of a critical section.
 */
void task_dealloc(task_t * task);

/**
 * Exits the current thread. This function enters critical sections itself.
 * Note that the final step of this call is to free the thread's kernel stack,
 * so the thread must first start using a CPU stack.
 * This function will automatically ensure that the current thread is a
 * "system" thread--that is, it can't be killed.
 */
void thread_exit();

