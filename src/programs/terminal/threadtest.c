#include <stdio.h>
#include <assert.h>
#include <pthread.h>

typedef struct {
  pthread_mutex_t mutex;
  uint64_t number;
} mutex_test_info;

static void test_basic();
static void * test_basic_thread(void * arg);

static void test_mutex();
static void * test_mutex_thread(void * arg);

void command_threadtest() {
  printf("testing basic pthreads... ");
  test_basic();
  test_basic();
  printf("passed!\n");
  printf("testing pthread_mutex... ");
  test_mutex();
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

static void test_mutex() {
  mutex_test_info info;
  pthread_mutex_init(&info.mutex, NULL);
  info.number = 25;

  pthread_t threadList[0x20];
  int i;
  for (i = 0; i < 1; i++) {
    int res = pthread_create(&threadList[i], NULL, test_mutex_thread, &info);
    printf("launched thread with ret value %x\n", res);
    assert(!res);
  }
  for (i = 0; i < 1; i++) {
    pthread_join(threadList[i], NULL);
  }
  printf("number is %d\n", info.number);
  assert(info.number == 1337);
  pthread_mutex_destroy(&info.mutex);
}

static void * test_mutex_thread(void * arg) {
  mutex_test_info * info = (mutex_test_info *)arg;
  printf("argument is %x\n", arg);
  int i;
  for (i = 0; i < 41; i++) {
    printf("trying to lock...\n");
    pthread_mutex_lock(&info->mutex);
    printf("locked it downnnnnnnn!\n");
    uint64_t number = info->number;
    sys_sleep(1);
    info->number = number + 1;
    pthread_mutex_unlock(&info->mutex);
  }
  return NULL;
}

