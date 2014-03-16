#include "types.h"
#include "gdt.h"

/**
 * Set the kernel stack in the TSS for our thread.
 */
void thread_configure_tss(thread_t * thread, tss_t * tss);

/**
 * Uses a thread's state to determine where in the kernpage address space the
 * next available byte of kernel stack is located.  If the process was in user
 * mode, this will be the first byte of kernel stack.  Otherwise, this will be
 * the program's last rsp value.
 * @discussion Obviously, this should be called from a critical section once
 * the thread has been marked as *running*.
 */
void * thread_resume_kernel_stack(task_t * task, thread_t * thread);

/**
 * Calculates the pointer in the task's address-space to a pointer that was
 * already in kernpage space. The pointer must be within the page allocated in
 * kernpage space for the thread's stack.
 */
void * thread_translate_kernel_stack(task_t * task,
                                     thread_t * thread,
                                     void * ptr);

/**
 * Call when an external IRQ is set.
 * @param irqMask = (1 << IRQ#)
 */
void scheduler_handle_interrupt(uint64_t irqMask);

