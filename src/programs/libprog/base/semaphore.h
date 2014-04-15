#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include <lock.h>

typedef struct {
  uint64_t lock;
  _th_queue_t * first;
  _th_queue_t * last;
  int64_t count;
} __attribute__((packed)) semaphore_t;

int semaphore_init(semaphore_t * psem, int64_t count);
int semaphore_destroy(semaphore_t * psem);
int semaphore_wait(semaphore_t * psem);
int semaphore_timedwait(semaphore_t * psem, uint64_t usec);
int semaphore_signal(semaphore_t * psem);

#endif
