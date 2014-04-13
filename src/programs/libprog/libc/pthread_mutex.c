#include <pthread_mutex.h>
#include <errno.h>
#include <strings.h>
#include <system.h>

int pthread_mutexattr_init(pthread_mutexattr_t * attr) {
  attr->type = 0;
  return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t * attr) {
  return 0;
}

int pthread_mutexattr_gettype(const pthread_mutexattr_t * attr, int * type) {
  *type = (int)attr->type;
  return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t * attr, int type) {
  if (type < 0 || type > 2) return EINVAL;
  attr->type = (int)type;
  return 0;
}

int pthread_mutex_init(pthread_mutex_t * mutex,
                       const pthread_mutexattr_t * attr) {
  bzero(mutex, sizeof(pthread_mutex_t));
  if (attr) mutex->type = attr->type;
  return 0;
}

int pthread_mutex_lock(pthread_mutex_t * mutex) {
  uint64_t threadId = sys_thread_id() + 1;
  if (mutex->holdingThread == threadId) {
    if (mutex->type != PTHREAD_MUTEX_RECURSIVE) {
      return EDEADLK;
    }
    mutex->holdingCount++;
    return 0;
  }

  basic_lock_lock(&mutex->lock);
  mutex->holdingThread = threadId;
  mutex->holdingCount = 0;
  return 0;
}

int pthread_mutex_unlock(pthread_mutex_t * mutex) {
  if (mutex->holdingThread != sys_thread_id() + 1) {
    return EPERM;
  }
  if (!--mutex->holdingCount) {
    mutex->holdingThread = 0;
    basic_lock_unlock(&mutex->lock);
  }
  return 0;
}

int pthread_mutex_destroy(pthread_mutex_t * mutex) {
  return 0;
}

