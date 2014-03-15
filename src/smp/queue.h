#include <stdint.h>

/**
 * The task queue is used to dispatch jobs to various CPU's
 */

typedef struct task_queue_item_t task_queue_item_t;

struct task_queue_item_t {
  void * thread;
  task_queue_item_t * next, * last;

  // 0 for normal threads, destination time for timer threads
  uint64_t firstTimestamp;
} __attribute__((packed));

typedef struct {
  task_queue_item_t * first, * last;
  uint64_t lock;
} __attribute((packed)) task_queue_t;

void task_queue_initialize();
void task_queue_lock();
void task_queue_unlock();

void task_queue_push(task_queue_item_t * item);
void task_queue_push_first(task_queue_item_t * item);
task_queue_item_t * task_queue_pop();
void task_queue_remove(task_queue_item_t * item);

