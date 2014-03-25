#include <anlock.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

static uint64_t number = 0;
static anlock_t lock = (anlock_t)&number;
static int threadCount = 10;

void * myMethod(void * number);
void myWaitMethod(void * ptr);

int main(int argc, const char * argv[]) {
  anlock_initialize(lock);
  threadCount = 10;
  for (uint32_t usec = 0; usec < 10; usec++) {
    pthread_t thread;
    uint64_t num = (uint64_t)(usec * 100000);
    pthread_create(&thread, NULL, myMethod, (void *)num);
  }
  while (1) { sleep(1); }
  return 0;
}

void * myMethod(void * ptr) {
  uint32_t number = (uint32_t)ptr;
  usleep(number);
  anlock_lock_waiting(lock, (void *)10000L, myWaitMethod);
  printf("I waited for %d useconds\n", (int)number);
  sleep(1);
  if (!--threadCount) {
    // make sure we can instantly seize and release the lock
    anlock_unlock(lock);
    anlock_lock(lock);
    anlock_unlock(lock);
    exit(0);
  }
  anlock_unlock(lock);
  return NULL;
}

void myWaitMethod(void * ptr) {
  usleep((uint32_t)ptr);
}
