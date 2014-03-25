#include <anscheduler/types.h>

#define MSR_STAR 0xC0000081
#define MSR_LSTAR 0xC0000082
#define MSR_SFMASK 0xC0000084

/**
 * Initialize the MSR's that never need to change again.
 */
void syscall_initialize();

/**
 * Configures a thread for syscalls. The thread's owning task must have a
 * reference to it.
 */
void syscall_initialize_thread(thread_t * thread);

/**
 * Setup the syscall handler the thread in question. The thread's owning task
 * must have a reference to it.
 */
void syscall_setup_for_thread(thread_t * thread);

