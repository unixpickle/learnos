/**
 * The cpu_ functions helps manage the CPU list and access CPU specific fields.
 */

#include "types.h"

/**
 * Adds a CPU to the CPU list. This does not lock anything, so this may not be
 * called after the CPU list has been populated and is being actively used.
 */
void cpu_add(cpu_t * cpu);

/**
 * Finds a CPU in the CPU list with a given APIC ID.
 */
cpu_t * cpu_lookup(uint32_t ident);

/**
 * Finds the CPU in the list with the current local APIC ID.
 */
cpu_t * cpu_current();

/**
 * Returns the pointer to the highest part of the CPU's dedicated kernel stack.
 */
void * cpu_dedicated_stack();

/**
 * Notifies every CPU running a task to task-switch early.
 */
void cpu_notify_task_dead(task_t * task);

