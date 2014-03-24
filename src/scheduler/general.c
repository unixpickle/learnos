#include "general.h"
#include <kernpage.h>
#include <anlock.h>
#include <stdio.h>

void * anscheduler_alloc(uint64_t size) {
  if (size > 0x1000) return NULL;
  kernpage_lock();
  void * ptr = (void *)(kernpage_alloc_virtual() << 12);
  kernpage_unlock();
  return ptr;
}

void anscheduler_free(void * buffer) {
  kernpage_lock();
  kernpage_free_virtual(((uint64_t)buffer) >> 12);
  kernpage_unlock();
}

void anscheduler_lock(uint64_t * ptr) {
  anlock_lock(ptr);
}

void anscheduler_unlock(uint64_t * ptr) {
  anlock_unlock(ptr);
}

void anscheduler_abort(const char * error) {
  // TODO: make this a thing
  print("[ABORT]: ");
  print(error);
  print("\n");
  __asm__("hlt");
}

void anscheduler_zero(void * buf, int len) {
  if (len >= 8) {
    uint64_t count = len >> 3;
    __asm__("mov $0, %%rax\n"
            "rep stosq"
            : "=D" (buf), "=c" (count)
            : "D" (buf), "c" (count)
            : "rax");
    len = len & 7;
  }
  for (; len > 0; len--) {
    (*(char *)(buf++)) = 0;
  }
}

void anscheduler_inc(uint64_t * ptr) {
  __asm__("incq (%0)" : : "r" (ptr));
}

void anscheduler_or_32(uint32_t * ptr, uint32_t flag) {
  __asm__("orl %0, (%1)" : : "r" (flag), "r" (ptr) : "memory");
}

