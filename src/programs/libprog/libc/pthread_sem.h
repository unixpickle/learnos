#ifndef __PTHREAD_SEM_H__
#define __PTHREAD_SEM_H__

#include <lock.h>

typedef struct {
  uint64_t lock;
  _th_queue_t * queue;
  int64_t count;
} __attribute__((packed)) pthread_sem_t;

int pthread_sem_init(pthread_sem_t * psem, int count);
int pthread_sem_destroy(pthread_sem_t * psem);
int pthread_sem_wait(pthread_sem_t * psem);
int pthread_sem_signal(pthread_sem_t * psem);

#endif
