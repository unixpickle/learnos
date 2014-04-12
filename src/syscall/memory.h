/**
 * This is a suite of syscalls which are generally only to be used by the
 * system pager. Some syscalls here will also be useful for hardware drivers
 * which need to allocate memory for hardware.
 */

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint64_t taskId;
  uint64_t threadId;
  uint64_t address;
  uint64_t flags;
} syscall_pg_t;

/**
 * Returns an allocated page in the kernel address space. The return value is a
 * normal pointer, not a page index or anything like that.
 * Requires root.
 * @discussion Only call this if you absolutely need to. If your task exits
 * before you have a chance to free this memory, it will leak forever!
 */
uint64_t syscall_allocate_page();

/**
 * Returns an allocated, aligned buffer in physical memory; use a length that is
 * a power of 2.
 * Requires root.
 * @discussion See discussion for syscall_allocate_page().
 */
uint64_t syscall_allocate_aligned(uint64_t pages);

/**
 * Free a single page at an address.
 */
void syscall_free_page(uint64_t addr);

/**
 * Free a buffer which was allocated with allocate_aligned().
 */
void syscall_free_aligned(uint64_t addr, uint64_t pages);

/**
 * Set an entry in a task's virtual memory table.
 * @param fd A link to the remote task.
 * @param vpage The virtual page index to map
 * @param entry The entry to set in the page table
 * @return true if the mapping was successfully set; false if the socket was
 * closed or if the mapping could not be made for any other reason.
 */
bool syscall_vmmap(uint64_t fd, uint64_t vpage, uint64_t entry);

/**
 * Completely unmap an entry from a task's virtual page table.
 * @param fd A link to the remote task.
 * @param vpage The virtual page index to unmap
 * @return false if the socket was closed; otherwise, true
 */
bool syscall_vmunmap(uint64_t fd, uint64_t vpage);

/**
 * Notify all CPUs running a certain task that its address space has been
 * altered.
 */
bool syscall_invlpg(uint64_t fd);

/**
 * Useful only to the system pager. This call returns the physical mapping for
 * a virtual page.
 */
uint64_t syscall_self_vm_read(uint64_t vpage);

/**
 * Tells the operating system that this process would like to assume the role of
 * the system memory manager.
 */
void syscall_become_pager();

/**
 * Get information on the next page fault that occurs. Returns 1 if a page fault
 * was read from the queue, or 0 if no faults were available. The fault is not
 * removed from the queue: a separate call to syscall_shift_fault() is needed.
 */
uint64_t syscall_get_fault(syscall_pg_t * pg);

/**
 * Reschedule a thread which was removed from the queue because of a page fault.
 * @param fd A link to the task.
 * @param tid The thread identifier (stack index) to wake
 * @return false if the task has died; true otherwise
 */
bool syscall_wake_thread(uint64_t fd, uint64_t tid);

/**
 * Map a page in our address space to a physical entry. This can only be called
 * by a task running as root.
 */
bool syscall_self_vmmap(uint64_t vpage, uint64_t entry);

/**
 * Completely unmap an entry from a task's virtual page table.
 * @return false if the task died.
 */
void syscall_self_vmunmap(uint64_t vpage);

/**
 * Notify all CPUs running a certain task that its address space has been
 * altered.
 */
void syscall_self_invlpg();

/**
 * Pops the first page fault from the queue.
 */
void syscall_shift_fault();

/**
 * Terminate a task, marking it as having died because of a memory fault. This
 * may only be used by the system pager.
 */
uint64_t syscall_mem_fault(uint64_t pid);

/**
 * Unmap a list of virtual addresses in a remote task's address space.
 * @return 0 if the socket is not open; 1 otherwise
 */
uint64_t syscall_batch_vmunmap(uint64_t fd, uint64_t start, uint64_t count);

/**
 * Batch allocate `count` pages and put pointers to their physical addresses in
 * the virtual stack address `listOut`.
 */
void syscall_batch_alloc(uint64_t listOut, uint64_t count);

