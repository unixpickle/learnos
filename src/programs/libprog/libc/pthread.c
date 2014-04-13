#include <pthread.h>
#include <errno.h>

static pthread_t * runningThreads __attribute__((aligned(8))) = NULL;
static uint64_t runningCount __attribute__((aligned(8))) = 0;
static basic_lock_t runningLock = BASIC_LOCK_INITIALIZER;

static void _push_thread(pthread_t thread);
static void _pop_thread(pthread_t thread);

int pthread_attr_init(pthread_attr_t * attr) {
  bzero(attr, sizeof(pthread_attr_t));
  return 0;
}

int pthread_attr_destroy(pthread_attr_t * attr) {
  return 0;
}

int pthread_attr_setdetachstate(pthread_attr_t * attr, int detach) {
  if (detach != 0 && detach != 1) return EINVAL;
  attr->detachState = (bool)detach;
  return 0;
}

int pthread_attr_getdetachstate(pthread_attr_t * attr, int * detach) {
  *detach = (int)attr->detachState;
  return 0;
}

int pthread_create(pthread_t * thread,
                   const pthread_attr_t * attr,
                   void * (* start_routine)(void *),
                   void * arg) {
  // TODO: create thread, push thread, run sys_launch_thread, return
}

int pthread_join(pthread_t thread, void ** retValue) {
  // TODO: poll for thread done
}

int pthread_detach(pthread_t thread) {
  // set thread isReferenced to false
  basic_lock_lock(&thread->lock);
  thread->isReferenced = false;
  basic_lock_unlock(&thread->lock);
}

pthread_t pthread_current() {
  basic_lock_lock(&runningLock);
  uint64_t i;
  uint64_t selfId = sys_thread_id();
  for (i = 0; i < runningCount; i++) {
    if (runningThreads[i]->threadId == selfId) {
      return runningThreads[i];
    }
  }
  basic_lock_unlock(&runningLock);
}

static void _push_thread(pthread_t thread) {
  basic_lock_lock(&runningLock);
  runningCount++;
  runningThreads = realloc(runningThreads, sizeof(pthread_t) * runningCount);
  runningThreads[runningCount - 1] = thread;
  basic_lock_unlock(&runningLock);
}

static void _pop_thread(pthread_t thread) {
  basic_lock_lock(&runningLock);
  if (runningCount == 1) {
    runningCount = 0;
    free(runningThreads);
    runningThreads = NULL;
    return;
  }

  bool found = false;
  uint64_t i;
  for (i = 0; i < runningCount; i++) {
    if (runningThreads[i] == thread) {
      found = true;
    } else if (found) {
      runningThreads[i - 1] = runningThreads[i];
    }
  }

  runningCount--;
  runningThreads = realloc(runningThreads, sizeof(pthread_t) * runningCount);

  basic_lock_unlock(&runningLock);
}

