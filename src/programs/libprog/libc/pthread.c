#include <pthread.h>
#include <errno.h>
#include <system.h>
#include <stdlib.h>
#include <strings.h>

static pthread_t * runningThreads __attribute__((aligned(8))) = NULL;
static uint64_t runningCount __attribute__((aligned(8))) = 0;
static basic_lock_t runningLock = BASIC_LOCK_INITIALIZER;

static void _push_thread(pthread_t thread);
static void _pop_thread(pthread_t thread);
static void _thread_entry(pthread_t thread);

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
  pthread_t val = malloc(sizeof(struct pthread));
  if (!val) return ENOMEM;
  bzero(val, sizeof(struct pthread));
  val->isReferenced = attr ? !attr->detachState : 1;
  val->isRunning = true;
  val->arg = arg;
  val->method = start_routine;
  sys_launch_thread((void (*)(void *))_thread_entry, (void *)val);
  if (thread) *thread = val;
  return 0;
}

int pthread_join(pthread_t thread, void ** retValue) {
  basic_lock_lock(&thread->lock);
  if (!thread->isRunning) {
    if (retValue) *retValue = thread->retValue;
    _pop_thread(thread);
    free(thread);
    return 0;
  }
  thread->isJoining = true;
  thread->joiningThread = sys_thread_id();
  basic_lock_unlock(&thread->lock);
  while (1) {
    sys_sleep(UINT64_MAX);
    basic_lock_lock(&thread->lock);
    if (!thread->isRunning) {
      if (retValue) *retValue = thread->retValue;
      _pop_thread(thread);
      free(thread);
      return 0;
    }
    basic_lock_unlock(&thread->lock);
  }
  // never reached
  return 0;
}

int pthread_detach(pthread_t thread) {
  basic_lock_lock(&thread->lock);
  thread->isReferenced = false;
  if (!thread->isRunning) {
    _pop_thread(thread);
    free(thread);
  } else {
    basic_lock_unlock(&thread->lock);
  }
  return 0;
}

void pthread_exit(void * value) {
  pthread_t cur = pthread_current();

  basic_lock_lock(&cur->lock);
  cur->retValue = value;
  if (!cur->isReferenced) {
    _pop_thread(cur);
    free(cur);
    sys_thread_exit();
  }

  cur->isRunning = false;
  if (cur->isJoining) {
    sys_unsleep(cur->joiningThread);
  }
  basic_lock_unlock(&cur->lock);

  sys_thread_exit();
}

pthread_t pthread_current() {
  basic_lock_lock(&runningLock);
  uint64_t i;
  uint64_t selfId = sys_thread_id();
  for (i = 0; i < runningCount; i++) {
    if (runningThreads[i]->threadId == selfId) {
      basic_lock_unlock(&runningLock);
      return runningThreads[i];
    }
  }
  basic_lock_unlock(&runningLock);
  return NULL;
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

static void _thread_entry(pthread_t thread) {
  thread->threadId = sys_thread_id();
  _push_thread(thread);
  void * val = thread->method(thread->arg);
  pthread_exit(val);
}

