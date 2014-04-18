#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>

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

typedef struct {
  sem_t semaphore;
  uint32_t count;
} sem_test_info;

static pthread_mutex_t overarchingLock __attribute__((aligned(8)))
  = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t overarchingCond __attribute__((aligned(8)));

static void * overarching_thread(void * arg);

static void test_basic();
static void * test_basic_thread(void * arg);

static void test_mutex();
static void * test_mutex_thread(void * arg);

static void test_condition_bcast();
static void test_condition_signal();
static void * test_condition_bcast_th(void * arg);
static void * test_condition_signal_th(void * arg);

static void test_semaphore();
static void * test_semaphore_th(void * arg);

void command_threadtest() {
  pthread_cond_init(&overarchingCond, NULL);
  pthread_t th1, th2;
  pthread_create(&th1, NULL, overarching_thread, NULL);
  pthread_create(&th2, NULL, overarching_thread, (void *)1);

  printf("testing basic pthreads... ");
  test_basic();
  test_basic(); // test that launching *another* thread works
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

  printf("testing semaphores... ");
  test_semaphore();
  printf("passed!\n");

  pthread_join(th1, NULL);
  pthread_cond_signal(&overarchingCond);
  pthread_join(th2, NULL);

  // otherwise things will happen...bad things...
  sys_exit();
}

static void * overarching_thread(void * arg) {
  if (!arg) return NULL;
  pthread_mutex_lock(&overarchingLock);
  pthread_cond_wait(&overarchingCond, &overarchingLock);
  return NULL;
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
    pthread_detach(threads[i]);
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

static void test_semaphore() {
  pthread_t threads[0x20];
  sem_test_info test = {0};
  int i;

  int res = sem_init(&test.semaphore, 0, 8);
  assert(!res);

  for (i = 0; i < 0x10; i++) {
    res = pthread_create(&threads[i], NULL, test_semaphore_th, (void *)&test);
    assert(!res);
  }
  uint64_t counts[8] = {0};
  for (i = 0; i < 0x10; i++) {
    void * ret = NULL;
    res = pthread_join(threads[i], &ret);
    assert(!res);
    uint64_t theCount = (uint64_t)ret;
    assert(theCount < 8);
    counts[theCount]++;
  }
  for (i = 0; i < 8; i++) {
    assert(counts[i] > 0);
  }
  sem_destroy(&test.semaphore);
  assert(test.count == 0);
}

static void * test_semaphore_th(void * arg) {
  sem_test_info * info = arg;
  int result = sem_wait(&info->semaphore);
  assert(!result);
  uint64_t res = (uint64_t)__sync_fetch_and_add(&info->count, 1);
  sys_sleep(0x10000);
  __sync_fetch_and_sub(&info->count, 1);
  sem_post(&info->semaphore);
  return (void *)res;
}

