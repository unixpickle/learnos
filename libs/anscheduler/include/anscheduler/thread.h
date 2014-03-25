#ifndef __ANSCHEDULER_THREAD_H__
#define __ANSCHEDULER_THREAD_H__

#include "types.h"

/**
 * Creates a thread and all its associated resources.
 * @param task A referenced task.
 * @return a new thread, or NULL if allocation failed.
 * @critical This doesn't take too much time because the user stack is
 * allocated lazily.
 */
thread_t * anscheduler_thread_create(task_t * task);

/**
 * Adds a thread to a task. This will schedule the thread, so make sure the
 * task has already been launched when you call this.
 * @param task A referenced task.
 * @param thread The thread to queue and begin executing.
 * @critical
 */
void anscheduler_thread_add(task_t * task, thread_t * thread);

/**
 * Set the thread to listen for events from sockets. If an event has already
 * been received, false is returned. Otherwise, true is returned.
 * @discussion You should save the state of the thread and switch to a CPU
 * stack before calling this.  However, if this doesn't return true, you may
 * jump right back into the thread in question.
 * @critical
 */
bool anscheduler_thread_poll();

/**
 * Call this to exit the current thread, presumably in a syscall handler.
 * @noncritical This potentially frees lots of memory, so it should be called
 * from a thread's execution state in kernel mode.
 */
void anscheduler_thread_exit();

/**
 * Deallocates a thread's user stack, locking the CPU when needed.
 * @noncritical This function can be interrupted or terminated at any time;
 * no memory will be leaked if proper cleanup is done later.
 */
void anscheduler_thread_deallocate(task_t * task, thread_t * thread);

/**
 * Returns the beginning of the kernel stack for a given thread or NULL if one
 * is not mapped.  The address returned is in the kernel's address space.
 * @critical
 */
void * anscheduler_thread_kernel_stack(task_t * task, thread_t * thread);

/**
 * Returns the address of the thread's kernel stack in the user's address
 * space. This address can be used to set the stack pointer after an
 * interrupt.
 * @critical
 */
void * anscheduler_thread_interrupt_stack(thread_t * thread);

/**
 * Returns the virtual address in the task's address space of a thread's user
 * stack. The thread's task must be referenced or, in some other way, you must
 * be sure that the thread will not be deallocated.
 * @critical
 */
void * anscheduler_thread_user_stack(thread_t * thread);

#endif
