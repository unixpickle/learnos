#include <stdio.h>
#include <assert.h>
#include <pthread.h>

typedef struct {
  pthread_mutex_t mutex;
  uint64_t number;
} mutex_test_info;

typedef struct {
  pthread_cond_t * cond;
  pthread_mutex_t * mutex;
  int count, numWaiting;
} cond_bcast_info;

typedef struct {
  pthread_cond_t * cond;
  pthread_mutex_t * mutex;
  int count;
  bool hasNotified;
} cond_signal_info;

static void test_basic();
static void * test_basic_thread(void * arg);

static void test_mutex();
static void * test_mutex_thread(void * arg);

static void test_condition_bcast();
static void * test_condition_bcast_th(void * arg);

static void test_condition_signal();
static void * test_condition_signal_th(void * arg);

void command_threadtest() {
  printf("testing basic pthreads... ");
  test_basic();
  test_basic();
  printf("passed!\n");
  printf("testing pthread_mutex... ");
  test_mutex();
  printf("passed!\n");
  printf("testing pthread_cond_broadcast()... ");
  test_condition_bcast();
  printf("passed!\n");
  printf("testing pthread_cond_signal()... ");
  test_condition_signal();
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
  for (i = 0; i < 0x20; i++) {
    int res = pthread_create(&threadList[i], NULL, test_mutex_thread, &info);
    assert(!res);
  }
  for (i = 0; i < 0x20; i++) {
    pthread_join(threadList[i], NULL);
  }
  assert(info.number == 1337);
  pthread_mutex_destroy(&info.mutex);
}

static void * test_mutex_thread(void * arg) {
  mutex_test_info * info = (mutex_test_info *)arg;
  int i;
  for (i = 0; i < 41; i++) {
    pthread_mutex_lock(&info->mutex);
    uint64_t number = info->number;
    sys_sleep(1);
    info->number = number + 1;
    pthread_mutex_unlock(&info->mutex);
  }
  return NULL;
}

static void test_condition_bcast() {
  pthread_cond_t cond;
  pthread_mutex_t lock;
  pthread_t threads[0x20];
  int i;

  pthread_cond_init(&cond, NULL);
  pthread_mutex_init(&lock, NULL);

  cond_bcast_info bcast = {&cond, &lock, 0, 0};
  for (i = 0; i < 0x20; i++) {
    int res = pthread_create(&threads[i], NULL,
                             test_condition_bcast_th,
                             (void *)&bcast);
    assert(!res);
  }
  while (1) {
    pthread_mutex_lock(&lock);
    int num = bcast.numWaiting;
    pthread_mutex_unlock(&lock);
    if (num == 0x20) break;
  }
  pthread_cond_broadcast(&cond);
  for (i = 0; i < 0x20; i++) {
    int res = pthread_join(threads[i], NULL);
    assert(!res);
  }
  assert(bcast.count == 0x20);
  assert(!cond.first);
  assert(!cond.last);

  pthread_mutex_destroy(&lock);
  pthread_cond_destroy(&cond);
}

static void test_condition_signal() {
  pthread_cond_t cond;
  pthread_mutex_t lock;
  pthread_t threads[0x20];
  int i;

  pthread_cond_init(&cond, NULL);
  pthread_mutex_init(&lock, NULL);

  cond_signal_info info = {&cond, &lock, 0, 0};
  for (i = 0; i < 0x20; i++) {
    int res = pthread_create(&threads[i], NULL,
                             test_condition_signal_th,
                             (void *)&info);
    assert(!res);
  }
  for (i = 0; i < 0x20; i++) {
    pthread_mutex_lock(&lock);
    info.hasNotified = 1;
    int initialCount = info.count;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);
    while (1) {
      pthread_mutex_lock(&lock);
      int theCount = info.count;
      pthread_mutex_unlock(&lock);
      if (theCount != initialCount) break;
      sys_sleep(1);
    }
  }

  assert(info.count == 0x20);
  pthread_mutex_destroy(&lock);
  pthread_cond_destroy(&cond);
}

static void * test_condition_bcast_th(void * arg) {
  cond_bcast_info * info = arg;
  pthread_mutex_lock(info->mutex);
  info->numWaiting++;
  pthread_cond_wait(info->cond, info->mutex);
  info->count++;
  pthread_mutex_unlock(info->mutex);
  return NULL;
}

static void * test_condition_signal_th(void * arg) {
  cond_signal_info * info = arg;
  pthread_mutex_lock(info->mutex);
  while (1) {
    if (info->hasNotified) {
      info->hasNotified = false;
      info->count++;
      break;
    }
    pthread_cond_wait(info->cond, info->mutex);
  }
  pthread_mutex_unlock(info->mutex);
  return NULL;
}

