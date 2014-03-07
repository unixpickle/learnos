#include "task.h"

/**
 * Call a function with a new stack pointer. This allows you to provide one
 * argument to the function.
 * @discussion Must be called from a critical section. The called function is
 * responsible for leaving the critical section.
 */
void task_run_with_stack(void * stack,
                         void * data,
                         void (* fn)(void * d)) __attribute__((noreturn));

/**
 * Load's a task's state registers and does a jump into the code. This will
 * not include an ltr instruction.
 * @discussion This must be called from a critical section, and will not
 * "return".
 */
void task_switch(thread_t * thread);

