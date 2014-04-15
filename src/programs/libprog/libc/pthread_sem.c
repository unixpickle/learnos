#include "pthread_sem.h"
#include <strings.h>
#include <anlock.h>
#include <errno.h>

int pthread_sem_init(pthread_sem_t * psem, int count) {
  return semaphore_init(psem, count);
}

int pthread_sem_destroy(pthread_sem_t * psem) {
  return semaphore_destroy(psem);
}

int pthread_sem_wait(pthread_sem_t * psem) {
  return semaphore_wait(psem);
}

int pthread_sem_signal(pthread_sem_t * psem) {
  return semaphore_signal(psem);
}

