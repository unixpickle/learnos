#include <stdbool.h>
#include <stdint.h>

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
 * @return true if the mapping was successfully set; false if the task died or
 * if the mapping could not be made for any other reason.
 */
bool syscall_vmmap(uint64_t pid, uint64_t vpage, uint64_t entry);

/**
 * Completely unmap an entry from a task's virtual page table.
 * @return false if the task died.
 */
bool syscall_vmunmap(uint64_t pid, uint64_t vpage);

/**
 * Notify all CPUs running a certain task that its address space has been
 * altered.
 */
bool syscall_invlpg(uint64_t pid);

