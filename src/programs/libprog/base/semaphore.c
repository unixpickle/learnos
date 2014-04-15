#include <semaphore.h>
#include <errno.h>
#include <strings.h>
#include <system.h>
#include <anlock.h>

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
  node.next = psem->queue;
  node.threadId = sys_thread_id();
  psem->queue = &node;
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
  node.next = psem->queue;
  node.threadId = sys_thread_id();
  psem->queue = &node;
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
  if (psem->count && psem->queue) {
    _th_queue_t * wakeup = psem->queue;
    psem->queue = wakeup->next;
    sys_unsleep(wakeup->threadId);
  }
  anlock_unlock(&psem->lock);
  return 0;
}

static bool _wait_or_pop(semaphore_t * sem, uint64_t usec, bool popOnFail) {
  sys_sleep(usec);
  uint64_t threadId = sys_thread_id();

  anlock_lock(&sem->lock);
  _th_queue_t * obj = sem->queue;
  _th_queue_t * last = NULL;
  while (obj) {
    if (obj->threadId == threadId) {
      if (popOnFail) {
        if (last) last->next = obj->next;
        else sem->queue = obj->next;
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

