
/**
 * Call a function with a new stack pointer. This allows you to provide one
 * argument to the function.
 * @discussion Must be called from a critical section. The called function is
 * responsible for leaving the critical section.
 */
void task_run_with_stack(void * stack,
                         void * data,
                         void (* fn)(void * d)) __attribute__((noreturn));

