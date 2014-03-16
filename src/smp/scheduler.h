#ifndef __TASK_SCHEDULER_H__
#define __TASK_SCHEDULER_H__

#include <stdint.h>
#include "types.h"

/**
 * Should be called after a LAPIC timer event. Never returns.
 */
void scheduler_handle_timer();

/**
 * Called whenever the LAPIC timer should be put back into the idle state.
 */
void scheduler_flush_timer();

/**
 * Returns the current global timestamp.
 */
uint64_t scheduler_get_timestamp();

/**
 * Disassociates the current thread with the current CPU. If the thread is not
 * in a special sleeping state, it will be added back to the end of the task
 * queue.
 */
void scheduler_stop_current();

/**
 * This will start scheduling the next thread.
 */
void scheduler_run_next();

/***********************
 * Task queue methods. *
 ***********************/

void task_queue_lock();
void task_queue_unlock();

void task_queue_push(thread_t * item);
void task_queue_push_first(thread_t * item);
thread_t * task_queue_pop();
void task_queue_remove(thread_t * item);

#endif
