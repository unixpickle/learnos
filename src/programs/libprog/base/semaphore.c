#include <semaphore.h>
#include <errno.h>
#include <strings.h>
#include <system.h>
#include <anlock.h>

static void _push_node(semaphore_t * sem, _th_queue_t * node);
static bool _wait_or_pop(semaphore_t * sem, uint64_t usec, bool popOnFail);

int semaphore_init(semaphore_t * psem, int64_t count) {
  bzero(psem, sizeof(semaphore_t));
  psem->count = count;
  return 0;
}

int semaphore_destroy(semaphore_t * psem) {
  return 0;
}

int semaphore_wait(semaphore_t * psem) {
  anlock_lock(&psem->lock);
  if (psem->count > 0) {
    psem->count--;
    anlock_unlock(&psem->lock);
    return 0;
  }
  // push our thread to the queue
  _th_queue_t node;
  node.next = NULL;
  node.threadId = sys_thread_id();
  _push_node(psem, &node);
  anlock_unlock(&psem->lock);
  while (!_wait_or_pop(psem, 0xffffffff, false));
  return 0;
}

int semaphore_timedwait(semaphore_t * psem, uint64_t usec) {
  anlock_lock(&psem->lock);
  if (psem->count > 0) {
    psem->count--;
    anlock_unlock(&psem->lock);
    return 0;
  }
  // push our thread to the queue
  _th_queue_t node;
  node.next = NULL;
  node.threadId = sys_thread_id();
  _push_node(psem, &node);
  anlock_unlock(&psem->lock);
  if (!_wait_or_pop(psem, usec, true)) {
    errno = ETIMEDOUT;
    return -1;
  }
  return 0;
}

int semaphore_signal(semaphore_t * psem) {
  anlock_lock(&psem->lock);
  psem->count++;
  if (psem->count && psem->first) {
    _th_queue_t * wakeup = psem->first;
    if (!(psem->first = wakeup->next)) {
      psem->last = NULL;
    }
    sys_unsleep(wakeup->threadId);
  }
  anlock_unlock(&psem->lock);
  return 0;
}

static void _push_node(semaphore_t * sem, _th_queue_t * node) {
  if (sem->last) {
    sem->last->next = node;
    sem->last = node;
  } else {
    sem->first = (sem->last = node);
  }
}

static bool _wait_or_pop(semaphore_t * sem, uint64_t usec, bool popOnFail) {
  sys_sleep(usec);
  uint64_t threadId = sys_thread_id();

  anlock_lock(&sem->lock);
  _th_queue_t * obj = sem->first;
  _th_queue_t * last = NULL;
  while (obj) {
    if (obj->threadId == threadId) {
      if (popOnFail) {
        if (last) last->next = obj->next;
        else if (!(sem->first = obj->next)) {
          sem->last = NULL;
        }
      }
      anlock_unlock(&sem->lock);
      return false;
    }
    last = obj;
    obj = obj->next;
  }
  anlock_unlock(&sem->lock);
  return true;
}

