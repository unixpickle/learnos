#include <anscheduler/types.h>

/**
 * Configures a thread to run a "user-space" function.
 */
void antest_configure_user_thread(thread_t * thread, void (* rip)());

/**
 * Triggers a page fault at a given address. This is useful, for instance,
 * for notifying the scheduler that a task has tried to access more stack.
 */
void antest_user_thread_page_fault(void * addr, bool write);

/**
 * Returns the virtual address (in the task's address space) of the top of the
 * current thread's stack.
 */
void * antest_user_thread_stack_addr();
