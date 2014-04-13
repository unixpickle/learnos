#include <pthread_mutex.h>
#include <errno.h>
#include <strings.h>

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

void pthread_mutex_init(pthread_mutex_t * mutex,
                        const pthread_mutexattr_t * attr) {
  bzero(mutex, sizeof(pthread_mutex_t));
}

#endif
