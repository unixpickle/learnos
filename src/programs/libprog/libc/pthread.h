#ifndef __PTHREAD_H__
#define __PTHREAD_H__

#include <stdint.h>
#include <stdbool.h>
#include <pthread_mutex.h>
#include <pthread_sem.h>
#include <pthread_cond.h>

#define PTHREAD_CREATE_JOINABLE 0
#define PTHREAD_CREATE_DETACHED 1

struct pthread {
  // only to be modified on the thread in question
  int errno, reserved1;
  void * arg;
  void * (* method)(void *);

  // fields locked by `lock`
  basic_lock_t lock;
  void * retValue;
  uint64_t threadId; // set to -1 on thread exit
  bool isRunning; // 1 when running, 0 when exited
  bool isReferenced; // 1 bedfore pthread_join() or pthread_detach()
  bool isJoining; // 0 before pthread_join(), 1 after
  char reserved2[5];
  uint64_t joiningThread; // the thread ID if isJoining is 1
} __attribute__((packed));

struct pthread_attr {
  bool detachState;
} __attribute__((packed));

typedef struct pthread * pthread_t;
typedef struct pthread_attr pthread_attr_t;

int pthread_attr_init(pthread_attr_t * attr);
int pthread_attr_destroy(pthread_attr_t * attr);
int pthread_attr_setdetachstate(pthread_attr_t * attr, int detach);
int pthread_attr_getdetachstate(pthread_attr_t * attr, int * detach);

int pthread_create(pthread_t * thread,
                   const pthread_attr_t * attr,
                   void * (* start_routine)(void *),
                   void * arg);
int pthread_join(pthread_t thread, void ** retValue);
int pthread_detach(pthread_t thread);
void pthread_exit(void * value);

pthread_t pthread_current();

#endif
