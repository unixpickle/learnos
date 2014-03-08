#include "task.h"
#include "gdt.h"

/**
 * Creates a thread which will first allocate a user stack for itself and then
 * will jump to user code pointed to by RIP.
 * @discussion This must be called from a critical section.
 */
thread_t * thread_create_user(task_t * task, void * rip);

/**
 * Creates the first thread for a new process. This thread copies in the user-
 * space code in addition to allocating a sure-space stack.
 * @discussion This much be called from a critical section.
 */
thread_t * thread_create_first(task_t * task,
                               void * program,
                               uint64_t len);

/**
 * Uses a thread's state to determine where in the kernpage address space the
 * next available byte of kernel stack is located.  If the process was in user
 * mode, this will be the first byte of kernel stack.  Otherwise, this will be
 * the programs last rsp value.
 * @discussion Obviously, this should be called from a critical section once
 * the thread has been marked as *running*.
 */
void * thread_resume_kernel_stack(task_t * task, thread_t * thread);

/**
 * Only to be called when the thread has already been cleaned up by a kernel
 * mini-program.
 */
void thread_dealloc(thread_t * thread);

/**
 * Sets the stack fields in the CSS.
 */
void thread_configure_tss(thread_t * thread, tss_t * tss);

/**
 * This method should never be called conventionally. Rather, it is a mini
 * program that runs when a new thread is started that initializes the
 * current thread's user-space stack buffer.
 */
void thread_configure_user_stack(void * rip);

/**
 * This method should never be called conventionally. Rather, it generates
 * the user space stack for the current thread, and then it copies in the
 * contents of the programs code to the program's address space.
 */
void thread_configure_user_program(void * rip, void * program, uint64_t len);

