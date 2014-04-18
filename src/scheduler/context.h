#include <anscheduler/types.h>

/**
 * Setup the task's TSS and then run thread_run_state.
 * @critical
 */
void anscheduler_thread_run(task_t * task, thread_t * thread);

/**
 * Configure a thread to run with interrupts enabled but CR3 set to the kernel
 * page table.
 * @critical
 */
void anscheduler_set_state(thread_t * thread,
                           void * stack,
                           void * ip,
                           void * arg1);

/**
 * Save the preserved registers and then call fn(arg).
 * @discussion Implemented in assembly.
 * @critical
 */
void anscheduler_save_return_state(thread_t * thread,
                                   void * arg,
                                   void (* fn)(void *));

/**
 * Gets the kernel stack to be used by thread_run_state()
 */
void * thread_resume_kernel_stack(thread_t * thread);

/**
 * Pass in a pointer that exists in a thread's page for its kernel stack. You
 * will get back a pointer to that same place in memory, just in the task's
 * address space.
 * @critical
 */
void * thread_vm_kernel_stack(thread_t * thread, void * ptr);

/**
 * Jump right into a thread's state. This will not set any entries in the TSS,
 * but it will load the needed registers.
 * @discussion Implemented in assembly.
 * @critical
 */
void thread_run_state(thread_t * thread);

/**
 * Switch to the kernel page tables. This is a bit difficult, so you will
 * probably want to use this in any syscall handlers.
 * @discussion Assembly
 * @critical
 */
void thread_switch_to_kernpage();

/**
 * Call this directly after an interrupt to save the state the task was in
 * before the interrupt was sent. The first argument is the number of 8-byte
 * fields have been pushed to the stack since the interrupt. This does not
 * include the 8-byte field which MUST be pushed right before calling this with
 * the old rdi value (since rdi is the arg register)
 * 
 * @discussion This automatically calls thread_switch_to_kernpage(). Written in
 * assembly.
 * @critical
 */
void thread_save_state(uint64_t fieldCount);

/**
 * Initialize the SSE state of a thread. This can be called from anywhere.
 * @critical or @noncritical
 */
void thread_fxstate_init(thread_t * thread);

