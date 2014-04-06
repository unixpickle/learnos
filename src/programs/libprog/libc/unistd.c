#include <unistd.h>
#include <stdbool.h>
#include <anlock.h>
#include <base/alloc.h>
#include <base/system.h>

static intptr_t brkSize __attribute__((aligned(8))) = 0;
static uint64_t brkLock __attribute__((aligned(8))) = 0;

static int _gain_memory(uintptr_t amount);
static int _lose_memory(uintptr_t amount);

int brk(void * addr) {
  anlock_lock(&brkLock);
  intptr_t len = (intptr_t)(ALLOC_DATA_BASE - addr);
  intptr_t getAmount = len - brkSize;
  if (getAmount < -brkSize) {
    anlock_unlock(&brkLock);
    return -1;
  }
  if (getAmount == 0) return 0;

  int result = 0;
  if (getAmount < 0) result = _lose_memory((uintptr_t)-getAmount);
  else result = _gain_memory(getAmount);
  anlock_unlock(&brkLock);
  return result;
}

void * sbrk(intptr_t increment) {
  anlock_lock(&brkLock);
  if (increment < -brkSize) {
    anlock_unlock(&brkLock);
    return (void *)(-1);
  }
  if (increment < 0) {
    if (_lose_memory((uintptr_t)-increment)) {
      anlock_unlock(&brkLock);
      return (void *)(-1);
    }
  } else {
    if (_gain_memory((uintptr_t)increment)) {
      anlock_unlock(&brkLock);
      return (void *)(-1);
    }
  }
  void * result = ALLOC_DATA_BASE + brkSize;
  anlock_unlock(&brkLock);
  return result;
}

void _exit(int unused) {
  sys_exit();
}

void exit(int unused) {
  sys_exit();
}

unsigned int sleep(unsigned int secs) {
  uint64_t start = sys_get_time();
  // sleep and then return the amount of time we missed
  sys_sleep(secs * 1000000);
  uint64_t delayed = sys_get_time() - start;
  return delayed / 1000000;
}

int usleep(useconds_t time) {
  sys_sleep(time);
  return 0;
}

static int _gain_memory(uintptr_t amount) {
  return -1;
}

static int _lose_memory(uintptr_t amount) {
  return -1;
}

