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
 * Get the next page fault, or NULL if no page faults are left to handle. Note
 * that, while the fault returned does include a referenced task, it will be
 * returned in a noncritical section. This is because it is presumed that the
 * system pager will NEVER die, and thus that the reference will not be
 * leaked.
 * @noncritical
 */
page_fault_t * anscheduler_pager_read();

#endif
