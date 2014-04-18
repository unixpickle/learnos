#include <semaphore.h>
#include <errno.h>
#include <assert.h>

int sem_init(sem_t * sem, int pshared, unsigned int value) {
  if (pshared) return ENOTSUP;
  pthread_mutex_init(&sem->mutex, NULL);
  pthread_cond_init(&sem->cond, NULL);
  sem->count = value;
  return 0;
}

int sem_destroy(sem_t * sem) {
  pthread_mutex_destroy(&sem->mutex);
  pthread_cond_destroy(&sem->cond);
  return 0;
}

int sem_wait(sem_t * sem) {
  pthread_mutex_lock(&sem->mutex);
  sem->count--;
  if (sem->count < 0) {
    int res = pthread_cond_wait(&sem->cond, &sem->mutex);
    assert(!res);
  }
  pthread_mutex_unlock(&sem->mutex);
  return 0;
}

int sem_trywait(sem_t * sem) {
  pthread_mutex_lock(&sem->mutex);
  if (sem->count <= 0) {
    pthread_mutex_unlock(&sem->mutex);
    return EAGAIN;
  }
  sem->count--;
  pthread_mutex_unlock(&sem->mutex);
  return 0;
}

int sem_post(sem_t * sem) {
  pthread_mutex_lock(&sem->mutex);
  if (++sem->count <= 0) {
    pthread_cond_signal(&sem->cond);
  }
  pthread_mutex_unlock(&sem->mutex);
  return 0;
}

int sem_getvalue(sem_t * sem, int * val) {
  pthread_mutex_lock(&sem->mutex);
  *val = (int)sem->count;
  pthread_mutex_unlock(&sem->mutex);
  return 0;
}

