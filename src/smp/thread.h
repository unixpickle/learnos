#include "task.h"

/**
 * Creates a thread which will first allocate a user stack for itself and then
 * will jump to user code pointed to by RIP.
 */
thread_t * thread_create_user(void * rip);

/**
 * Creates the first thread for a new process. This thread copies in the user-
 * space code in addition to allocating a sure-space stack.
 */
thread_t * thread_create_first(void * rip, void * program, uint64_t len);

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

