#include "task.h"

/**
 * Start executing a new thread on this CPU, or sleep if there are no events to
 * be processed.
 * @discussion This must be called from within a critical section.
 */
void scheduler_resign();

/**
 * Run the current thread from a new state.
 * @discussion This must be called from within a critical section.
 */
void scheduler_jump_state(struct state_t * newState);

/**
 * Jump to a specific task. The current task's state must be saved in order for
 * this to work.
 */
void scheduler_switch_task(task_t * task, thread_t * thread);

