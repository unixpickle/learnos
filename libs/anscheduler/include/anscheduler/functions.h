#ifndef __ANSCHEDULER_FUNCTIONS_H__
#define __ANSCHEDULER_FUNCTIONS_H__

/**
 * These are platform-specific methods which must be implemented by whatever
 * platform you wish to use anscheduler on. It is expected that an
 * implementation for each of these functions be present at runtime.
 */

#include "types.h"

#define ANSCHEDULER_PAGE_FLAG_PRESENT 1
#define ANSCHEDULER_PAGE_FLAG_WRITE 2
#define ANSCHEDULER_PAGE_FLAG_USER 4
#define ANSCHEDULER_PAGE_FLAG_GLOBAL 0x100
#define ANSCHEDULER_PAGE_FLAG_UNALLOC 0x200
#define ANSCHEDULER_PAGE_FLAG_SWAPPED 0x400

/*******************
 * General Purpose *
 *******************/

/**
 * Allocates at least `size` bytes. For kernels with page-sized allocators
 * only, this should return NULL if `size` is greater than the maximum page
 * size. Additionally, this address must be aligned on a 4KB page boundary.
 * @return Address to the beginning of the new buffer, or NULL on failure.
 * @critical
 */
void * anscheduler_alloc(uint64_t size);

/**
 * Frees a buffer allocated with anscheduler_alloc().
 * @critical
 */
void anscheduler_free(void * buffer);

/**
 * Locks a spinlock of some sort which should be represented by 64-bits.
 * @critical
 */
void anscheduler_lock(uint64_t * ptr);

/**
 * Unlocks something locked with anscheduler_lock()
 * @critical
 */
void anscheduler_unlock(uint64_t * ptr);

/**
 * Terminates application or kernel flow.
 * @critical
 */
void anscheduler_abort(const char * error);

/**
 * Zero a chunk of memory. This is an external function because some CPUs may
 * have an optimized way of doing this.
 * @noncritical or @critical
 */
void anscheduler_zero(void * buf, int len);

/**
 * Atomically increment the value at a memory address.
 * @critical
 */
void anscheduler_inc(uint64_t * ptr);

/**
 * Atomically OR a 32-bit value at a memory address with a specified value.
 */
void anscheduler_or_32(uint32_t * ptr, uint32_t flag);

/*********************
 * Context Switching *
 *********************/

/**
 * @param task Must have a reference to it.
 * @param thread The thread whose state to pick up
 * @critical
 */
void anscheduler_thread_run(task_t * task, thread_t * thread);

/**
 * Configures a thread to run a kernel job in the kernel's address space.
 * @param thread The thread whose state to configure.
 * @param stack The stack which the thread should run under
 * @param ip A pointer to the code to be executed.
 * @param arg The first pointer argument to pass to the function.
 */
void anscheduler_set_state(thread_t * thread,
                           void * stack,
                           void * ip,
                           void * arg1);

/**
 * This function never returns. Instead, it saves the state if it were to
 * return to a thread. Then, it calls a specified function and passes a
 * pointer argument.
 * @param thread The thread whose state to store into
 * @param arg The argument to pass to the following function.
 * @param fn A function which should NEVER RETURN
 * @critical
 */
void anscheduler_save_return_state(thread_t * thread,
                                   void * arg,
                                   void (* fn)(void *));

/*********
 * Timer *
 *********/

/**
 * Make a tick occur in at least `ticks` ticks.
 * @critical
 */
void anscheduler_timer_set(uint32_t ticks);

/**
 * Set a slow timer. Call this before doing long operations in scheduling
 * functions in order to ensure that the system clock stays calibrated.
 * @critical
 */
void anscheduler_timer_set_far();

/**
 * Cancel whatever timer has been set.
 * @critical
 */
void anscheduler_timer_cancel();

/**
 * Get the current timestamp, in ticks.
 */
uint64_t anscheduler_get_time();

/**
 * Get the number of ticks in one second.
 */
uint64_t anscheduler_second_length();

/***************
 * CPU Context *
 ***************/

/**
 * Enter a critical section.
 * @noncricital Why would you call this if not from a noncritical section?
 */
void anscheduler_cpu_lock();

/**
 * Leave a critical section.
 * @critical You should only unlock the CPU if you have a reason to!.
 */
void anscheduler_cpu_unlock();

/**
 * @critical
 */
task_t * anscheduler_cpu_get_task();

/**
 * @critical
 */
thread_t * anscheduler_cpu_get_thread();

/**
 * @critical
 */
void anscheduler_cpu_set_task(task_t * task);

/**
 * @critical
 */
void anscheduler_cpu_set_thread(thread_t * thread);

/**
 * Notify every CPU running a specified task that the task's virtual memory
 * mapping has been modified.
 * @critical
 */
void anscheduler_cpu_notify_invlpg(task_t * task);

/**
 * Notify every CPU running a task to switch tasks. This may be as simple as
 * triggering an early timer tick on every CPU running the task.
 * @critical
 */
void anscheduler_cpu_notify_dead(task_t * task);

/**
 * Calls a function `fn` with an argument `arg` using a stack dedicated to
 * this CPU. This is useful for the last part of any thread kill, when the
 * thread's own kernel stack must be freed.
 * @critical
 */
void anscheduler_cpu_stack_run(void * arg, void (* fn)(void * a));

/**
 * @noncritical Calling from a critical section would hang this CPU.
 */
void anscheduler_cpu_halt(); // wait until timer or interrupt

/******************
 * Virtual Memory *
 ******************/

/**
 * @noncritical or @critical This should be relatively simple and static.
 */
uint64_t anscheduler_vm_physical(uint64_t virt); // global VM translation

/**
 * @noncritical or @critical See anscheduler_vm_physical().
 */
uint64_t anscheduler_vm_virtual(uint64_t phys); // global VM translation

/**
 * Create a root page table entry. On x86-64, this is a PML4.
 * @critical
 */
void * anscheduler_vm_root_alloc();

/**
 * Map a virtual page to a physical page and set flags on the page.
 * @return false if the operation failed (i.e. a page table could not be
 * allocated).
 * @critical
 */
bool anscheduler_vm_map(void * root,
                        uint64_t vpage,
                        uint64_t dpage,
                        uint16_t flags);
                        
/**
 * Unmap a virtual page in a virtual memory mapping.
 * @critical
 */
void anscheduler_vm_unmap(void * root, uint64_t vpage);

/**
 * Lookup the physical entry and flags for a given virtual page. If the page
 * is not mapped, 0 should be returned along with 0 flags.
 * @critical
 */
uint64_t anscheduler_vm_lookup(void * root,
                               uint64_t vpage,
                               uint16_t * flags);

/**
 * This will only be called early on if very little memory has been mapped
 * Like anscheduler_vm_root_free_async(), but made to run in critical sections.
 * @critical 
 */
void anscheduler_vm_root_free(void * root);

/**
 * Free a virtual memory mapping. This should not assume that all memory has
 * been unmapped. Be prepared to do a recursive memory free here.
 * @noncritical Every time you free something, you should enter a critical
 * section. You may have to unmap a lot of stuff, so your method should be
 * interruptable.
 */
void anscheduler_vm_root_free_async(void * root);

/*********
 * Hooks *
 *********/

/**
 * Called from a kernel thread when a task is about to be freed. At this point,
 * the task's virtual memory mapping will still be in tact, but it will have no
 * sockets open and no external references to it. If you have no task-specific
 * data to free here, you need not do anything in this implementation.
 * @noncritical
 */
void anscheduler_task_cleanup(task_t * task);

#endif

