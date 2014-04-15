#ifndef __PTHREAD_SEM_H__
#define __PTHREAD_SEM_H__

#include <semaphore.h>

typedef semaphore_t pthread_sem_t;

int pthread_sem_init(pthread_sem_t * psem, int count);
int pthread_sem_destroy(pthread_sem_t * psem);
int pthread_sem_wait(pthread_sem_t * psem);
int pthread_sem_signal(pthread_sem_t * psem);

#endif
