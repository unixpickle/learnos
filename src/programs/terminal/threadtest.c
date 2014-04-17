#include <stdio.h>
#include <assert.h>
#include <pthread.h>

static void test_basic();
static void * test_basic_thread(void * arg);

void command_threadtest() {
  printf("testing basic pthreads... ");
  test_basic();
  printf("passed!\n");
  sys_exit();
}

static void test_basic() {
  pthread_t thread;
  int ret = pthread_create(&thread, NULL, test_basic_thread, (void *)0x1337);
  assert(!ret);
  void * retVal = NULL;
  ret = pthread_join(thread, &retVal);
  assert(!ret);
  assert(retVal == (void *)~0x1337L);
}

static void * test_basic_thread(void * arg) {
  return (void *)~(uint64_t)arg;
}

