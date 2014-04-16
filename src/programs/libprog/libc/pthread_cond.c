#include "pthread_cond.h"
#include <system.h>
#include <strings.h>

static bool _cond_contains(pthread_cond_t * cond);

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
  while (1) {
    sys_sleep(UINT64_MAX);
    pthread_mutex_lock(m);
    if (!_cond_contains(c)) break;
    pthread_mutex_unlock(m);
  }
  return 0;
}

int pthread_cond_signal(pthread_cond_t * c) {
  if (c->first) {
    _th_queue_t * first = c->first;
    if (!(c->first = first->next)) {
      c->last = NULL;
    }
    sys_unsleep(first->threadId);
  }
  return 0;
}

int pthread_cond_broadcast(pthread_cond_t * c) {
  while (c->first) {
    sys_unsleep(c->first->threadId);
    c->first = c->first->next;
  }
  c->first = (c->last = NULL);
  return 0;
}

static bool _cond_contains(pthread_cond_t * cond) {
  _th_queue_t * node = cond->first;
  while (node) {
    if (node->threadId == sys_thread_id()) {
      return true;
    }
    node = node->next;
  }
  return false;
}

