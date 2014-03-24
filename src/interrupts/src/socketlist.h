#include <anscheduler/types.h>

/**
 * @critical
 */
void anscheduler_descriptor_set(task_t * task, socket_desc_t * desc);

/**
 * @critical
 */
void anscheduler_descriptor_delete(task_t * task, socket_desc_t * desc);

/**
 * @critical
 */
socket_desc_t * anscheduler_descriptor_find(task_t * task, uint64_t desc);

/**
 * @critical
 */
void anscheduler_task_pending(task_t * task, socket_desc_t * desc);

/**
 * @critical
 */
void anscheduler_task_not_pending(task_t * task, socket_desc_t * desc);

/**
 * @return A referenced socket descriptor.
 * @critical
 */
socket_desc_t * anscheduler_task_pop_pending(task_t * task);
