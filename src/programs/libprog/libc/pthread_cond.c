#include "pthread_cond.h"

typedef struct {
  _th_queue_t * first;
  _th_queue_t * last;
} __attribute__((packed)) pthread_cond_t;

int pthread_condattr_init(pthread_condattr_t * attr) {
  return 0;
}

int pthread_condattr_destroy(pthread_condattr_t * attr) {
  return 0;
}

int pthread_cond_init(pthread_cond_t * c, pthread_condattr_t * attr) {
  bzero(c, sizeof(pthread_cond_t));
  return 0;
}

int pthread_cond_wait(pthread_cond_t * c, pthread_mutex_t * m) {
  // put ourselves in the queue
  _th_queue_t th;
  th.next = NULL;
  th.threadId = sys_thread_id();
  if (c->last) {
    c->last->next = &th;
    c->last = &th;
  } else {
    c->last = (c->first = &th);
  }
  pthread_mutex_unlock(m);
}

int pthread_cond_signal(pthread_cond_t * c);
int pthread_cond_broadcast(pthread_cond_t * c);

