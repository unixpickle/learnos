#ifndef __ANSCHEDULER_LOOP_H__
#define __ANSCHEDULER_LOOP_H__

#include "types.h"

/**
 * Pushes the current thread back to the run loop for another time. This must
 * be called before running anscheduler_loop_run() function. However,
 * anscheduler_loop_switch() calls this automatically.
 *
 * @discussion If you are calling this, you must be sure that you have already
 * switched to a CPU-specific kernel stack that is not associated  with the
 * thread; otherwise, the next CPU running the thread could smash our stack.
 *
 * @critical
 */
void anscheduler_loop_push_cur();

/**
 * Removes a thread from the run loop if it is in the queue at all.
 * @param thread The thread which has been killed.
 */
void anscheduler_loop_delete(thread_t * thread);

/**
 * Used to add a thread to the scheduling queue.
 */
void anscheduler_loop_push(thread_t * newThread);

/**
 * Enters the scheduling loop.  This function should never return.  By this
 * point, you should be on the CPU dedicated stack.
 * @critical
 */
void anscheduler_loop_run();

/**
 * Enters the scheduling loop. You do NOT have to call this from the CPU
 * stack.  If you call this without somehow pushing your current thread, the
 * thread will not be pushed back to the scheduling loop and may never be run
 * again (unless you set isPolling = true).
 * @critical
 */
void anscheduler_loop_break_task();

/**
 * Resign early from this time slice.  This can be called from any stack. It
 * is equivalent to switching to the CPU stack and then calling both
 * anscheduler_loop_push_cur() and anscheduler_loop_run();
 * @critical
 */
void anscheduler_loop_resign();

/**
 * Saves the current thread and then calls anscheduler_loop_resign(). This will
 * return when the thread runs again.
 * @critical
 */
void anscheduler_loop_save_and_resign();

/**
 * Add a kernel thread to the scheduler queue. The kernel thread must not exit
 * via anscheduler_thread_exit(), but rather through 
 * anscheduler_loop_delete_cur_kernel()
 * @critical
 */
void anscheduler_loop_push_kernel(void * arg, void (* fn)(void * arg));

/**
 * Deletes the current kernel thread, freeing its memory and not adding it
 * back to the queue.
 */
void anscheduler_loop_delete_cur_kernel();

/**
 * Switches from this thread to a different thread.  In order to call this
 * method, you must have already set isPolling back to 0 in the new thtread so
 * that no other task will attempt to switch into it.
 * This method switches to the CPU dedicated stack for you.
 * @param task A referenced task
 * @param thread The thread in the task
 * @critical
 */
void anscheduler_loop_switch(task_t * task, thread_t * thread);

#endif
