#include <stdint.h>

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

