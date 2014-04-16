#ifndef __SYSCALL_VM_H__
#define __SYSCALL_VM_H__

#include <stdint.h>
#include <stdbool.h>

/**
 * Copies the contents at a virtual memory address in the current task's address
 * to a pointer in *our* address space. Returns false if the specified task ptr
 * is invalid in some way (i.e. covers unmapped pages, swapped pages, etc.).
 * @critical
 */
bool task_copy_in(void * kPointer, const void * tPointer, uint64_t len);

/**
 * Copies the contents of a kernel virtual address to a task's address space.
 * The copy operation will fail if the task doesn't have write access to the
 * memory.
 * @critical
 */
bool task_copy_out(void * tPointer, const void * kPointer, uint64_t len);

/**
 * Gets the virtual address in kernel space for the user-space address. This is
 * only guaranteed to be valid on a page boundary, so you must verify the
 * alignment of your address in advance.
 * @param tPtr The task pointer
 * @param outPtr The output pointer in kernel space
 * @return true if the address could be found; false otherwise
 */
bool task_get_virtual(const void * tPtr, void ** ourPtr);

#endif
