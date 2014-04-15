#include "pthread_sem.h"
#include <strings.h>
#include <anlock.h>
#include <errno.h>

int pthread_sem_init(pthread_sem_t * psem, int count) {
  bzero(psem, sizeof(pthread_sem_t));
  return 0;
}

int pthread_sem_destroy(pthread_sem_t * psem) {
  return 0;
}

int pthread_sem_wait(pthread_sem_t * psem) {
  anlock_lock(&psem->lock);
  if (psem->count > 0) {
    psem->count--;
    anlock_unlock(&psem->lock);
    return 0;
  }
  anlock_unlock(&psem->lock);
  // TODO: start waiting for the semaphore
  return EDEADLK; // too lazy
}

int pthread_sem_signal(pthread_sem_t * psem) {
  anlock_lock(&psem->lock);
  psem->count++;
  anlock_unlock(&psem->lock);
  return 0;
}

