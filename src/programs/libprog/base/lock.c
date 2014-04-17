#include "lock.h"
#include "system.h"
#include <anlock.h>

/**
 * Returns `true` if the lock was able to be locked, false if added but waiting.
 */
static bool _lock_try_or_add(_th_queue_t * queue, basic_lock_t * lock);

/**
 * Go through each thread in the waiting queue and see if we are one of them. If
 * we are, `false` is returned, otherwise `true`.
 * @param lock The lock.
 * @param remove If `true`, if this thread is found, remove it from the queue
 * before returning `false`.
 */
static bool _lock_check_or_have(basic_lock_t * lock, bool remove);

void basic_lock_lock(basic_lock_t * lock) {
  _th_queue_t thisQueue = {0, sys_thread_id()};
  if (_lock_try_or_add(&thisQueue, lock)) return;
  while (!_lock_check_or_have(lock, false)) {
    // this thread will be woken on unlock anyway
    sys_sleep(0xffffffffffffffff);
  }
}

bool basic_lock_timedlock(basic_lock_t * lock, uint64_t micros) {
  sys_clear_unsleep(); // prevent a spurrious wakeup

  _th_queue_t thisQueue = {0, sys_thread_id()};
  if (_lock_try_or_add(&thisQueue, lock)) return true;
  sys_sleep(micros);

  return _lock_check_or_have(lock, true);
}

void basic_lock_unlock(basic_lock_t * lock) {
  anlock_lock(&lock->lock);
  if (!lock->first) {
    lock->isLocked = 0;
    anlock_unlock(&lock->lock);
    return;
  }
  _th_queue_t * th = lock->first;
  if (!(lock->first = th->next)) {
    lock->last = NULL;
  }
  sys_unsleep(th->threadId);
  anlock_unlock(&lock->lock);
}

static bool _lock_try_or_add(_th_queue_t * queue, basic_lock_t * lock) {
  anlock_lock(&lock->lock);
  if (!lock->isLocked) {
    lock->isLocked = 1;
    anlock_unlock(&lock->lock);
    return true;
  }
  // push ourself to the waiting queue
  if (lock->last) {
    lock->last = (lock->last->next = queue);
  } else {
    lock->first = (lock->last = queue);
  }
  anlock_unlock(&lock->lock);
  return false;
}

static bool _lock_check_or_have(basic_lock_t * lock, bool remove) {
  uint64_t selfId = sys_thread_id();
  anlock_lock(&lock->lock);
  _th_queue_t * aNode = lock->first;
  _th_queue_t * last = NULL;
  while (aNode) {
    if (aNode->threadId == selfId) {
      // remove it, unlock the lock, and return false
      if (remove) {
        if (last) last->next = aNode->next;
        else lock->first = aNode->next;
        if (!aNode->next) lock->last = last;
      }
      anlock_unlock(&lock->lock);
      return false;
    }
    last = aNode;
    aNode = aNode->next;
  }
  anlock_unlock(&lock->lock);
  return true;
}

