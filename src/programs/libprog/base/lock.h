#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct _th_queue_t _th_queue_t;

struct _th_queue_t {
  _th_queue_t * next;
  uint64_t threadId;
} __attribute__((packed));

typedef struct {
  _th_queue_t * first;
  _th_queue_t * last;
  uint64_t isLocked;
  uint64_t lock;
} __attribute__((packed)) basic_lock_t;

#define BASIC_LOCK_INITIALIZER {NULL, NULL, 0, 0}

void basic_lock_lock(basic_lock_t * lock);
bool basic_lock_timedlock(basic_lock_t * lock, uint64_t micros);
void basic_lock_unlock(basic_lock_t * lock);

