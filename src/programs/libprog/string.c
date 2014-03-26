#include "string.h"

void memcpy(void * dest, const void * src, uint64_t len) {
  uint8_t * _d = dest;
  const uint8_t * _s = src;
  while (len--) {
    (*(_d++)) = *(_s++);
  }
}

uint64_t strlen(const char * str) {
  uint64_t res = 0;
  while ((str++)[0]) res++;
  return res;
}

bool strequal(const char * str1, const char * str2) {
  while ((*str1) && (*str2)) {
    if ((*(str1++)) != (*(str2++))) return false;
  }
  return ((*str1) == (*str2));
}

