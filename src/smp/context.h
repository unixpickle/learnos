#include "types.h"

/**
 * Call this directly after an interrupt is received to save the current CPU
 * state to the current thread's data structure.
 * @discussion This must be called from a critical section. Note: this function
 * switches to the kernpage address space automatically and keeps the same
 * stack page just like task_switch_to_kernpage().
 */
void task_save_state();

/**
 * Call this anywhere from a critical section in the execution context of a
 * thread and save the state as it would be upon returning from the call.
 * @return 0 if this is a normal return, 1 if this return occurred later on as
 * a result of a task switch.
 */
uint64_t task_save_caller_state();

/**
 * Call this soon after receiving an interrupt if the handler does not call
 * task_state_save() in order to switch to the kernel page tables.
 */
void task_switch_to_kernpage();

/**
 * Load's a task's state registers and does a jump into the code. This will
 * not include an ltr instruction.
 * @discussion This must be called from a critical section, and will not
 * "return".
 */
void task_switch(task_t * task, thread_t * thread);

/**
 * Handler for the LAPIC timer interrupt that triggers a task switch.
 */
void task_switch_interrupt();

