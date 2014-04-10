#ifndef __ANSCHEDULER_PAGING_H__
#define __ANSCHEDULER_PAGING_H__

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
 * Get the system thread which is responsible for handling non-trivial
 * application page faults.
 * @critical
 */
thread_t * anscheduler_pager_get();

/**
 * Set the system thread which is responsible for handling non-trivial
 * application page faults.
 * @critical
 */
void anscheduler_pager_set(thread_t * thread);

/**
 * Remove the top page fault from the list of faults.
 * @critical
 */
void anscheduler_pager_shift();

/**
 * Returns the next page fault without popping it from the queue. This should be
 * called using a system call, which should then use anscheduler_pager_read to
 * suck away the last fault.
 * @critical
 */
page_fault_t * anscheduler_pager_peek();

/**
 * Global lock for the pager functions. You usually do not have to call this
 * yourself.
 */
void anscheduler_pager_lock();

/**
 * See anscheduler_pager_lock().
 */
void anscheduler_pager_unlock();

/**
 * Call this after anscheduler_pager_lock(). As of now, this is the only function
 * that will not call anscheduler_pager_lock() for you.
 * @return true if page faults are waiting in the queue; otherwise, false.
 */
bool anscheduler_pager_waiting();

#endif
