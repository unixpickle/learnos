#ifndef __PTHREAD_COND_H__
#define __PTHREAD_COND_H__

#include "pthread_mutex.h"

#define PTHREAD_COND_INITIALIZER {NULL, NULL, 0}

typedef struct {
  _th_queue_t * first;
  _th_queue_t * last;
  uint64_t fieldLock;
} __attribute__((packed)) pthread_cond_t;

typedef struct {
} __attribute__((packed)) pthread_condattr_t;

int pthread_condattr_init(pthread_condattr_t * attr);
int pthread_condattr_destroy(pthread_condattr_t * attr);

int pthread_cond_init(pthread_cond_t * c, pthread_condattr_t * attr);
int pthread_cond_wait(pthread_cond_t * c, pthread_mutex_t * m);
int pthread_cond_signal(pthread_cond_t * c);
int pthread_cond_broadcast(pthread_cond_t * c);
int pthread_cond_destroy(pthread_cond_t * c);

#endif
