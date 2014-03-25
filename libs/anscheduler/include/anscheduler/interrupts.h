#ifndef __ANSCHEDULER_INTERRUPTS_H__
#define __ANSCHEDULER_INTERRUPTS_H__

#include "types.h"

#define ANSCHEDULER_PAGE_FAULT_PRESENT 1
#define ANSCHEDULER_PAGE_FAULT_WRITE 2
#define ANSCHEDULER_PAGE_FAULT_USER 4
#define ANSCHEDULER_PAGE_FAULT_INSTRUCTION 0x10

/**
 * Call this whenever a page fault or platform-equivalent interrupt occurs.
 *
 * This function should never return. Instead, it should switch directly
 * back into the current task if it wishes to proceed running. Otherwise, it
 * should call back to the run loop.
 * @critical
 */
void anscheduler_page_fault(void * ptr, uint64_t flags);

/**
 * Call this whenever an external IRQ comes in. When the IRQ arrives, save
 * the state of the current task. Then, if this function returns, restore the
 * state of the current task and continue running it. This is because this
 * function always has the possibility of not returning, but it could also
 * return if the interrupt thread is working on something.
 * @critical
 */
void anscheduler_irq(uint8_t irqNumber);

/**
 * @return The current thread which is registered to receive interrupts.
 */
thread_t * anscheduler_interrupt_thread();

/**
 * Set the current thread which will receive interrupts.
 */
void anscheduler_set_interrupt_thread(thread_t * thread);

/**
 * If the passed thread is the current interrupt thread, set the interrupt
 * thread to NULL. This should be called whenever a thread dies.
 */
void anscheduler_interrupt_thread_cmpnull(thread_t * thread);

#endif
