#ifndef __SCHEDULER_CODE_H__
#define __SCHEDULER_CODE_H__

typedef struct code_t code_t;

#include <anscheduler/types.h>

struct code_t {
  uint64_t retainCount; // atomic

  // points to the actual kernpage linear code data
  void * kernpageBase;
  uint64_t kernpageLen;

  void ** pageTables[0xffd]; // NULL = no table
} __attribute__((packed));

/**
 * Allocate a new, empty code structure.
 * @critical
 */
code_t * code_allocate(void * base, uint64_t len);

/**
 * Retain a code structure (i.e. for a new task or something).
 * @critical
 */
code_t * code_retain(code_t * code);

/**
 * Release a code structure and free its memory if the retain count has reached
 * zero.
 * @noncritical
 */
void code_release(code_t * code);

/**
 * Allocate the designated page which triggered a fault. When this has finished,
 * it will return `true` or `false`. If `true` is returned, the task should be
 * resumed. Otherwise, it should be terminated.
 * @critical
 */
bool code_handle_page_fault(code_t * code,
                            thread_t * thread,
                            void * ptr,
                            uint64_t flags);

/**
 * Called to cleanup the code section of a task. This will go through the VM map
 * and free the code pages which are marked as writable. Additionally, this will
 * automatically call code_release(code).
 */
void code_task_cleanup(code_t * code, task_t * task);

#endif

