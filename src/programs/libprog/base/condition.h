#include <lock.h>

typedef struct {
  uint64_t lock;
  _th_queue_t * queue;
} __attribute__((packed)) condition_t;



