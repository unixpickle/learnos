#ifndef __ANSCHEDULER_INTERRUPTS_H__
#define __ANSCHEDULER_INTERRUPTS_H__

#include "types.h"

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
 * @critical
 */
thread_t * anscheduler_intd_get();

/**
 * Set the current thread which will receive interrupts.
 * @critical
 */
void anscheduler_intd_set(thread_t * thread);

/**
 * If the passed thread is the current interrupt thread, set the interrupt
 * thread to NULL. This should be called whenever a thread dies.
 * @critical
 */
void anscheduler_intd_cmpnull(thread_t * thread);

/**
 * Reads the current interrupt mask and sets it back to zero.
 * @critical
 */
uint32_t anscheduler_intd_read();

#endif
