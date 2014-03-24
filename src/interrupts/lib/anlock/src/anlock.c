#include "anlock.h"

static uint64_t read_value_atomically(uint64_t * ptr);

void anlock_initialize(anlock_t lock) {
  *lock = 0;
}

void anlock_lock(anlock_t lock) {
  anlock_lock_waiting(lock, (void *)0, (void *)0);
}

void anlock_lock_waiting(anlock_t lock, void * data, void (*fn)(void * d)) {
  volatile anlock_t ptr = lock;
  uint64_t oldValue = __sync_fetch_and_add(ptr, 1);
  uint32_t lower = (uint32_t)(oldValue & 0xffffffffL);
  if (!lower) return; // we have seized the lock first!
  
  uint32_t upper = (uint32_t)(oldValue >> 32L);
  uint32_t waitUntil = (uint32_t)(upper + lower);
  while (1) {
    uint64_t nowUpper = read_value_atomically(lock) >> 32L;
    if (nowUpper == (uint64_t)waitUntil) return;
    if (fn) fn(data);
  }
}

void anlock_unlock(anlock_t lock) {
  // use Intel's `lock` directive here to ensure that the unlock
  // operation is atomic.
  __asm__ __volatile__ ("movq $0xffffffff, %%rax\n"
                        "lock addq %%rax, (%%rcx)"
                        : // no output
                        : "c"(lock)
                        : "rax", "memory");
}

static uint64_t read_value_atomically(uint64_t * ptr) {
  return __sync_fetch_and_add(ptr, 0);
}
