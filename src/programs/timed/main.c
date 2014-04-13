#include <stdio.h>
#include <pthread.h>
#include <assert.h>

void * background_thread(void * arg);

void main() {
  sys_sleep(0x100000);

  sys_print("[timed]: launching thread.\n");
  pthread_t thread;
  int res = pthread_create(&thread, NULL, background_thread, (void *)1);
  assert(!res);
  sys_print("[timed]: joining thread.\n");
  void * ret = NULL;
  res = pthread_join(thread, &ret);
  assert(!res);
  assert(ret == (void *)2);
  sys_print("[timed]: test passed.\n");
}

void * background_thread(void * arg) {
  sys_print("[timed]: in background thread\n");
  assert(arg == (void *)1);
  return (void *)2;
}

