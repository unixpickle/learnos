#include "task.h"

/**
 * Jump to a specific task. The current task's state must be saved in order for
 * this to work.
 * @param task - The task to launch.
 * @param thread - The thread to launch. Note that this must already have
 * isRunning set to true.
 * @return This function will never return.
 * @discussion This must be run from a critical section.
 */
void scheduler_switch_task(task_t * task, thread_t * thread);

/**
 * Called (usually by an interrupt) when the current CPU would like to run a new
 * task.
 * @return If this function returns, that means no available tasks where found.
 * If this happens, this function is responsible for setting up a timer to wake
 * it up in the future to look for a task again.
 * @discussion This must be called from a critical section.
 */
void scheduler_run_next();

/**
 * Creates a new task with one thread and adds it to the task queue.
 * @discussion This must be called from a critical section.
 */
bool scheduler_generate_task(void * code, uint64_t len);

/**
 * Call this when an external IRQ comes in.
 */
void scheduler_handle_interrupt(uint64_t irqMask);

