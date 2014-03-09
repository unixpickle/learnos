#include "task.h"

/**
 * @discussion You must lock the task's PML4 manually. This function must be run
 * from within a critical section.
 */
page_t task_vm_lookup(task_t * task, page_t virt);

/**
 * @discussion see task_vm_lookup() discussion
 */
bool task_vm_set(task_t * task, page_t virt, uint64_t value);

/**
 * @discussion see task_vm_lookup() discussion
 */
void task_vm_make_user(task_t * task, page_t virt);

/**
 * @discussion see task_vm_lookup() discussion
 */
void task_vm_unmap(task_t * task, page_t virt);

/**
 * Finds the address in the task's address space that corresponds to an address
 * in the kernpage tables.
 * @discussion This must be called from a critical section, but it will lock
 * the PML4 tables for you.
 */
void * task_vm_get_from_kernpage(task_t * task, void * ptr);

