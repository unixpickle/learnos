#include "general.h"
#include <string.h>
#include <stdio.h>
#include <anlock.h>

void anscheduler_lock(uint64_t * ptr) {
  anlock_lock(ptr);
}

void anscheduler_unlock(uint64_t * ptr) {
  anlock_unlock(ptr);
}

void anscheduler_abort(const char * error) {
  fprintf(stderr, "[fatal]: %s\n", error);
  exit(1);
}

void anscheduler_zero(void * buf, int len) {
  bzero(buf, len);
}

void anscheduler_inc(uint64_t * ptr) {
  __asm__("incq (%0)" : : "r" (ptr) : "memory");
}

void anscheduler_or_32(uint32_t * ptr, uint32_t flag) {
  __asm__("orl %0, (%1)" : : "r" (flag), "r" (ptr) : "memory");
}