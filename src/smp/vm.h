#include "types.h"

/**
 * @discussion You must lock the task's PML4 manually. This function must be run
 * from within a critical section.
 */
page_t task_vm_lookup(task_t * task, page_t virt);

/**
 * Same as task_vm_lookup, but finds the raw entry in the table.
 */
uint64_t task_vm_lookup_raw(task_t * task, page_t virt);

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

